/*
* File Name: xmcs_gpon.c
* Description: GPON I/O Command Process for xPON Management Control 
*			  Subsystem
*
******************************************************************/
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include "common/drv_global.h"
#include "gpon/gpon_dev.h"
#include "gpon/gpon_dvt.h"
#include "xmcs/xmcs_gpon.h"
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HYBIRD)
#include "pwan/hybird_wan.h"
#endif/*TCSUPPORT_COMPILE*/
#include <ecnt_hook/ecnt_hook_fe.h>

#define GPON_MAX_SD_THRESHOLD   9
#define GPON_MIN_SD_THRESHOLD   6
#define GPON_MAX_SF_THRESHOLD   8
#define GPON_MIN_SF_THRESHOLD   5

int distanceAdjust = 0;

/***************************************************************
***************************************************************/
int xmcs_set_sn_passwd(struct XMCS_GponSnPasswd_S *pSnPasswd) 
{
	memset(gpGponPriv->gponCfg.sn, 0, GPON_SN_LENS) ;
	memcpy(gpGponPriv->gponCfg.sn, pSnPasswd->sn, GPON_SN_LENS) ;
	
	memset(gpGponPriv->gponCfg.passwd, 0, GPON_PASSWD_LENS) ;
	memcpy(gpGponPriv->gponCfg.passwd, pSnPasswd->passwd,GPON_PASSWD_LENS) ;
    gpGponPriv->gponCfg.PasswdLength=pSnPasswd->PasswdLength;
    gpGponPriv->gponCfg.hexFlag=pSnPasswd->hexFlag;
#if/*TCSUPPORT_COMPILE*/ !(defined(TCSUPPORT_XPON_HAL_API) || defined(TCSUPPORT_XPON_HAL_API_EXT))
    gpGponPriv->emergencystate = pSnPasswd->EmergencyState;
#endif/*TCSUPPORT_COMPILE*/

	PON_MSG(MSG_TRACE, "SN:%s, PASSWD:%s\n", pSnPasswd->sn, pSnPasswd->passwd) ;
	
	return 0 ;
}
		
static int xmcs_set_sn(struct XMCS_GponSnPasswd_S *pSnPasswd) 
{
	memset(gpGponPriv->gponCfg.sn, 0, GPON_SN_LENS) ;
	memcpy(gpGponPriv->gponCfg.sn, pSnPasswd->sn, GPON_SN_LENS) ;
	
	return 0 ;
}

static int xmcs_set_passwd(struct XMCS_GponSnPasswd_S *pSnPasswd) 
{
	memset(gpGponPriv->gponCfg.passwd, 0, GPON_PASSWD_LENS) ;
	memcpy(gpGponPriv->gponCfg.passwd, pSnPasswd->passwd, GPON_PASSWD_LENS) ;
	
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
	gpGponPriv->gponCfg.to1Timer = pActTimer->to1Timer ;
	gpGponPriv->to1_timer.expires = gpGponPriv->gponCfg.to1Timer ;

	gpGponPriv->gponCfg.to2Timer = pActTimer->to2Timer ;
	gpGponPriv->to2_timer.expires = gpGponPriv->gponCfg.to2Timer ;
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_set_act_timer_to1(uint to1Timer) 
{
	gpGponPriv->gponCfg.to1Timer = to1Timer ;
	gpGponPriv->to1_timer.expires = gpGponPriv->gponCfg.to1Timer ;

	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_set_act_timer_to2(uint to2Timer) 
{
	gpGponPriv->gponCfg.to2Timer = to2Timer ;
	gpGponPriv->to2_timer.expires = gpGponPriv->gponCfg.to2Timer ;

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
int xmcs_get_onu_info(struct XMCS_GponOnuInfo_S *pOnuInfo) 
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
#ifndef TCSUPPORT_CPU_EN7521
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
	
	if(pTrtcmParams->channel>=XPON_CHANNEL_NUMBER) {
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
	QDMA_TxDbaReport_T gponTrtcmParams = {0};
#else
	QDMA_TcontTrtcm_T gponTrtcmParams = {0};
	int ret = 0 ;
#endif
	
	
	if(pTrtcmParams->channel>=XPON_CHANNEL_NUMBER) {
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

static int xmcs_set_gpon_tx_rate_limit(struct XMCS_GponTxRateLimit_S * pTxRateLimit)
{
	QDMA_TxRateLimitSet_T gponTxRateLimit;
	if(pTxRateLimit == NULL)
		return -1;
	if(pTxRateLimit->chnlIdx > 31)
		return -1;
	if(pTxRateLimit->chnlRateLimitEn != 0 && pTxRateLimit->chnlRateLimitEn != 1)
		return -1;
	gponTxRateLimit.chnlIdx = pTxRateLimit->chnlIdx;
	gponTxRateLimit.chnlRateLimitEn = pTxRateLimit->chnlRateLimitEn;
	gponTxRateLimit.rateLimitValue = pTxRateLimit->rateLimitValue;
	return QDMA_API_SET_TX_RATELIMIT(ECNT_QDMA_WAN,&gponTxRateLimit);
}

static int xmcs_get_gpon_tcont_info(struct XMCS_GponTcontInfo_S * pTcontInfo)
{
	int ret = 0;
	ret = gponDevGetTcontInfo(pTcontInfo->allocId,&(pTcontInfo->tcontIdx));
	if(0 != ret)
    {
		return -EINVAL ;
    }
	return ret ;
}
/***************************************************************
***************************************************************/
static int xmcs_set_gpon_tod(struct XMCS_GponTodCfg_S *pGponTod) 
{
#ifdef TCSUPPORT_CPU_EN7581
	gponDevSetNewTod(*pGponTod) ;
#else
	gponDevSetNewTod(pGponTod->superframe, pGponTod->sec, pGponTod->nanosec) ;
#endif
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
int get_counter_from_reg(STATISTIC_CFG_t *pGemCouter, unchar readClear)
{
    ushort gemIdx = 0;
    GWAN_GemInfo_T * gemInfo = NULL;
    ushort portid = 0;

	if(pGemCouter->gemPortId >= GPON_MAX_GEM_ID)
	{
		printk("gemport id = %d is invalid, not greater %d\n", pGemCouter->gemPortId, GPON_MAX_GEM_ID);
		return -1;
	}

	gemIdx = (gpWanPriv->gpon.gemIdToIndex[pGemCouter->gemPortId] & GPON_GEM_IDX_MASK);
	gemInfo = & gpWanPriv->gpon.gemPort[gemIdx].info;
	portid = gemInfo->portId;
	
    if(!gemInfo->valid){
        return -1;
	}
    

	if( 0!= gponDevGetGemPortCounter(portid, GEMPORT_RX_FRAME_CNT,   &pGemCouter->sta.rxGemFrame, readClear) )
        return -1;
    
	if( 0!= gponDevGetGemPortCounter(portid, GEMPORT_RX_PL_BYTE_CNT, &pGemCouter->sta.rxGemPayload, readClear) )
        return -1;
    
	if( 0!= gponDevGetGemPortCounter(portid, GEMPORT_TX_FRAME_CNT,   &pGemCouter->sta.txGemFrame, readClear) )
        return -1;
    
	if( 0!= gponDevGetGemPortCounter(portid, GEMPORT_TX_PL_BYTE_CNT, &pGemCouter->sta.txGemPayload, readClear) )
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
int xmcs_get_gem_counter(STATISTIC_CFG_t *pGponGemPortCount) 
{
	int ret = 0;
	ret = get_counter_from_reg(pGponGemPortCount, 0);
    if(0!=ret)
    {
		return -EINVAL ;
    }
	return ret ;
}


/***************************************************************
***************************************************************/
int xmcs_get_gem_counter_clear(STATISTIC_CFG_t *pGponGemPortCount) 
{
	int ret = 0;
	ret = get_counter_from_reg(pGponGemPortCount,1);
    if(0!=ret)
    {
		return -EINVAL ;
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
			    get_counter_from_reg(&gemTemp, 0);
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
static int xmcs_get_activation_counter(unsigned int *counter)
{
	*counter = gpGponPriv->activationCnt;
	return 0;
}
/***************************************************************
***************************************************************/
static int xmcs_get_gponAlarm(GPON_Alarm_T * alarmInfo)
{
	*alarmInfo = gpGponPriv->gponAlarm;
	return 0;
}
/***************************************************************
***************************************************************/
static int xmcs_get_ploamMsg_counter(GPON_PLOAM_CNT_T* counter)
{
	*counter= gpGponPriv->ploamMsgcounter;
	return 0;
}
/***************************************************************
	***************************************************************/
static int xmcs_get_omciRX_counter(GPON_OMCI_CNT_T *counter)
{
	counter->rxOmciMsgCnt = gpWanPriv->gpon.rx_omci_cnt;
	return 0;
}
/***************************************************************
	***************************************************************/
static int xmcs_get_omciRX_Base_counter(GPON_OMCI_CNT_T *counter)
{
	counter->rxOmciMsgCnt = gpWanPriv->gpon.rx_omci_cnt  \
                            - gpWanPriv->gpon.rx_omci_extend_cnt;
	return 0;
}
/***************************************************************
	***************************************************************/
static int xmcs_get_omciRX_Extend_counter(GPON_OMCI_CNT_T *counter)
{
	counter->rxOmciMsgCnt = gpWanPriv->gpon.rx_omci_extend_cnt ;
	return 0;
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
#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7528)
static int xmcs_set_dying_gasp_intvl(uint intvl) 
{
	return gponDevSetDyingGaspIntvl(intvl);
}

/***************************************************************
***************************************************************/
static int xmcs_get_dying_gasp_intvl(uint *intvl) 
{
	return gponDevGetDyingGaspIntvl(intvl);
}
#endif

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
        gpon_recover_delete_gemport(pReset->gemPort[pReset->gemPortNum-1].gemPortId);

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

#ifdef TCSUPPORT_CPU_EN7521
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
    FE_RxCnt_t rxCnt = {0};
    FE_API_GET_ETH_RX_CNT(&rxCnt);
	gponGetCounter->gponGetCounter_table[0] = 0;                                      
	gponGetCounter->gponGetCounter_table[1] = 2;
	gponGetCounter->gponGetCounter_table[2] = rxCnt.rxDropCnt;
	gponGetCounter->gponGetCounter_table[3] = rxCnt.rxFrameLen;
	gponGetCounter->gponGetCounter_table[4] = rxCnt.rxFrameCnt;
	gponGetCounter->gponGetCounter_table[5] = rxCnt.rxBroadcastCnt;            
	gponGetCounter->gponGetCounter_table[6] = rxCnt.rxMulticastCnt;
	gponGetCounter->gponGetCounter_table[7] = rxCnt.rxCrcCnt;
	gponGetCounter->gponGetCounter_table[8] = rxCnt.rxLess64Cnt;
	gponGetCounter->gponGetCounter_table[9] = rxCnt.rxMore1518Cnt;
	gponGetCounter->gponGetCounter_table[10] = rxCnt.rxEq64Cnt;
	gponGetCounter->gponGetCounter_table[11] = rxCnt.rxFrom65To127Cnt;
	gponGetCounter->gponGetCounter_table[12] = rxCnt.rxFrom128To255Cnt;
	gponGetCounter->gponGetCounter_table[13] = rxCnt.rxFrom256To511Cnt;
	gponGetCounter->gponGetCounter_table[14] = rxCnt.rxFrom512To1023Cnt;
	gponGetCounter->gponGetCounter_table[15] = rxCnt.rxFrom1024To1518Cnt;
	return 0;
}
/***************************************************************
***************************************************************/
static int xmcs_get_gpon_traffic_state(uint *arg)
{
    if( GPON_TRAFFIC_UP == gwan_get_traffic_status() )
	{
		*arg = 0;
	}
	else
	{
		*arg = 1;
	}
	return 0;
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
		*arg = 0;	
	}
	else
	{
		*arg = 1;
	}
	return 0;
}
#endif /* TCSUPPORT_CPU_EN7521 */

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HYBIRD)
static int xmcs_add_hybird_route_mac(GPON_HYBIRD_ROUTE_MAC_T * pRouteMac) 
{
    GPON_HYBIRD_MAC_ACTION_TYPE_t type = HYBIRD_ROUTE_MAC_ADD_ACTION;
	return hybird_pwan_set_route_mac(pRouteMac,type);
}

static int xmcs_del_hybird_route_mac(GPON_HYBIRD_ROUTE_MAC_T * pRouteMac) 
{
    GPON_HYBIRD_MAC_ACTION_TYPE_t type = HYBIRD_ROUTE_MAC_DEL_ACTION;
	return hybird_pwan_set_route_mac(pRouteMac,type);
}

#endif/*TCSUPPORT_COMPILE*/

/***************************************************************
***************************************************************/
#if 0
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
#endif

static int xmcs_set_mib_flag(uint arg) 
{
	gpGponPriv->pGponRecovery->mibFlag = arg;
	if(gpGponPriv->pGponRecovery->mibFlag){	
		stop_omci_oam_monitor();		
		gpGponPriv->gpon_traffic_status = TRAFFIC_DOWN;
		xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_TRAFFIC_STATUS_CHANGE, 0) ;
		start_omci_oam_monitor();
	}
	return 0 ;
}

static int xmcs_set_SD_SF_threshold(GPON_DEV_SD_SF_THLD_T * thld) 
{
    if( 0 != thld->sd && (thld->sd > GPON_MAX_SD_THRESHOLD || thld->sd < GPON_MIN_SD_THRESHOLD)){
        printk("input error SD threshold should in 4..9 SD[%d]\n",thld->sd);
        return -1;
    }

    if( 0 != thld->sf && (thld->sf > GPON_MAX_SF_THRESHOLD || thld->sf < GPON_MIN_SF_THRESHOLD)){
        printk("input error SF threshold should in 3..8 SF[%d]\n",thld->sf);
        return -1;
    }

    if( (0 != thld->sd && 0 != thld->sf) && (thld->sd <= thld->sf)){
        printk("input error SD/SF ,threshold SD should bigger than SF , now input SD[%d] SF[%d]\n",thld->sd,thld->sf);
        return -1;
    }

    if(0 != thld->sd){
        gGpon_SD_SF_Info.SD_thld = thld->sd;
	}
	
    if(0 != thld->sf){
		gGpon_SD_SF_Info.SF_thld = thld->sf;
	}
	return 0 ;
}

static int xmcs_get_SD_SF_threshold(GPON_DEV_SD_SF_THLD_T * thld) 
{
    thld->sd = gGpon_SD_SF_Info.SD_thld;
    thld->sf = gGpon_SD_SF_Info.SF_thld;
    
	return 0 ;
}


static int xmcs_get_SD_SF_cnt(GPON_DEV_SD_SF_CNT_T * cnt) 
{
    cnt->sd= gGpon_SD_SF_Info.SD_cnt;
    cnt->sf = gGpon_SD_SF_Info.SF_cnt;
    if(1 == cnt->cl_en){
        gGpon_SD_SF_Info.SD_cnt = 0;
        gGpon_SD_SF_Info.SF_cnt = 0;
    }
	
	return 0 ;
}

int xmcs_clear_tcont_counter(ushort allocID) 
{
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
			if(allocID == gemInfo->info[i].allocId)
			{
				gponDevClearGemPortCounter(gemInfo->info[i].gemPortId) ;             
			}
		}
	}
	if(gemInfo != NULL)
	{
	    kfree(gemInfo);
	}
    return 0 ;
}

int xmcs_get_olt_distance(uint32_t * oltDistance)
{
    REG_G_EQD gponEqd;
    uint32_t eqd = 0;
    uint32_t distance = 0;
	int hw5800_base = 0;

    gponEqd.Raw = IO_GREG(G_EQD);
    eqd = gponEqd.Bits.eqd;
	if(oltDistance == NULL){
		return -1;
	}
    /** FD = (TEqd - EqD)*102  TEqd = 250us ; Eqd = regVal/1244
     *  (250-regVal/1244) *102 
     */
    PON_MSG(MSG_XMCS,"eqd=%#x(%d),response time=%#x(%d)\n",
    		eqd,eqd,gpGponPriv->gponCfg.onuResponseTime,gpGponPriv->gponCfg.onuResponseTime);
			
    distance = 250*102 - (eqd+gpGponPriv->gponCfg.onuResponseTime) * 102/1244;
	if(distance < 1000){
		hw5800_base = 15;
	}else{
		hw5800_base = 15 + (distance-1000)*2/1000;
	}	
	PON_MSG(MSG_XMCS,"auto adjust by hw5800 base = %d\n", hw5800_base);
    *oltDistance = distance + hw5800_base + distanceAdjust;
   	PON_MSG(MSG_XMCS,"onu cal olt distance = %u\n", *oltDistance);
    return 0;
}


/***************************************************************
***************************************************************/

int gpon_cmd_proc(uint cmd, ulong arg) 
{
	int ret = -1;
	
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
			{
				struct XMCS_GponSnPasswd_S temp;
				memset(&temp, 0, sizeof(struct XMCS_GponSnPasswd_S)) ;
				COPY_FROM_USER(&temp,(struct XMCS_GponSnPasswd_S *)arg,sizeof(struct XMCS_GponSnPasswd_S),ret);
				ret = xmcs_set_sn(&temp) ;
			}
			break ;
		case GPON_IOS_PASSWD :
			{
				struct XMCS_GponSnPasswd_S temp;
				memset(&temp, 0, sizeof(struct XMCS_GponSnPasswd_S)) ;				
				COPY_FROM_USER(&temp,(struct XMCS_GponSnPasswd_S *)arg,sizeof(struct XMCS_GponSnPasswd_S),ret);
				ret = xmcs_set_passwd(&temp) ;
			}
			break ;
		case GPON_IOS_EMERGENCY_STATE :
			ret = xmcs_set_emergency_state((XPON_Mode_t)arg) ;
			break;
		case GPON_IOS_ACT_TIMER :
			{
				struct XMCS_GponActTimer_S temp = {0};
				COPY_FROM_USER(&temp,(struct XMCS_GponActTimer_S *)arg,sizeof(struct XMCS_GponActTimer_S),ret);
				ret = xmcs_set_act_timer(&temp) ;
			}
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
			{
				ushort temp = 0;
				ret = xmcs_get_idle_gem(&temp);
				COPY_TO_USER((ushort *)arg,&temp,sizeof(ushort),ret);
			}
			break ;
		case GPON_IOG_ONU_INFO :
			{
				struct XMCS_GponOnuInfo_S temp = {0};
				COPY_FROM_USER(&temp,(struct XMCS_GponOnuInfo_S *)arg,sizeof(struct XMCS_GponOnuInfo_S),ret);
			    ret = xmcs_get_onu_info(&temp) ;
				COPY_TO_USER((struct XMCS_GponOnuInfo_S *)arg,&temp,sizeof(struct XMCS_GponOnuInfo_S),ret);
			}
			break ;
		case GPON_IOS_DBA_BLOCK_SIZE :
			ret = xmcs_set_gpon_block_size((ushort)arg) ;
			break ;
		case GPON_IOG_DBA_BLOCK_SIZE :
			{
				ushort temp = 0;
				ret = xmcs_get_gpon_block_size(&temp);
				COPY_TO_USER((ushort *)arg,&temp,sizeof(ushort),ret);
			}
			break ;
		case GPON_IOS_GPON_TRTCM_MODE : 
			{
				struct XMCS_GponTrtcmConfig_S temp = {0};
				COPY_FROM_USER(&temp,(struct XMCS_GponTrtcmConfig_S *)arg,sizeof(struct XMCS_GponTrtcmConfig_S),ret);
				ret = xmcs_set_gpon_trtcm_mode(&temp) ;
			}
			break ;
		case GPON_IOS_GPON_TRTCM_SCALE : 
			{
				struct XMCS_GponTrtcmConfig_S temp = {0};
				COPY_FROM_USER(&temp,(struct XMCS_GponTrtcmConfig_S *)arg,sizeof(struct XMCS_GponTrtcmConfig_S),ret);
				ret = xmcs_set_gpon_trtcm_scale(&temp) ;
			}
			break ;
		case GPON_IOG_GPON_TRTCM_CONFIG :
			{
				struct XMCS_GponTrtcmConfig_S temp = {0};
				COPY_FROM_USER(&temp,(struct XMCS_GponTrtcmConfig_S *)arg,sizeof(struct XMCS_GponTrtcmConfig_S),ret);
			    ret = xmcs_get_gpon_trtcm_config(&temp) ;
				COPY_TO_USER((struct XMCS_GponTrtcmConfig_S *)arg,&temp,sizeof(struct XMCS_GponTrtcmConfig_S),ret);				
			}
			break ;
		case GPON_IOS_GPON_TRTCM_PARAMS :
			{
				struct XMCS_GponTrtcmParams_S temp = {0};
				COPY_FROM_USER(&temp,(struct XMCS_GponTrtcmParams_S *)arg,sizeof(struct XMCS_GponTrtcmParams_S),ret);
				ret = xmcs_set_gpon_trtcm_params(&temp) ;
			}
			break ;
		case GPON_IOG_GPON_TRTCM_PARAMS :
			{
				struct XMCS_GponTrtcmParams_S temp = {0};
				COPY_FROM_USER(&temp,(struct XMCS_GponTrtcmParams_S *)arg,sizeof(struct XMCS_GponTrtcmParams_S),ret);			
			    ret = xmcs_get_gpon_trtcm_params(&temp) ;
				COPY_TO_USER((struct XMCS_GponTrtcmParams_S *)arg,&temp,sizeof(struct XMCS_GponTrtcmParams_S),ret);				
			}
			break ;
		case GPON_IOS_GPON_TX_RATE_LIMIT:
			{
				struct XMCS_GponTxRateLimit_S temp = {0};
				COPY_FROM_USER(&temp,(struct XMCS_GponTxRateLimit_S *)arg,sizeof(struct XMCS_GponTxRateLimit_S),ret);				
				ret = xmcs_set_gpon_tx_rate_limit((struct XMCS_GponTxRateLimit_S *)arg);
			}
			break;
		case GPON_IOG_GPON_TCONT_IDX:
			{
				struct XMCS_GponTcontInfo_S temp = {0};
				COPY_FROM_USER(&temp,(struct XMCS_GponTcontInfo_S *)arg,sizeof(struct XMCS_GponTcontInfo_S),ret);						
			    ret = xmcs_get_gpon_tcont_info(&temp) ;
				COPY_TO_USER((struct XMCS_GponTcontInfo_S *)arg,&temp,sizeof(struct XMCS_GponTcontInfo_S),ret);				
			}
			break;
		case GPON_IOS_TOD_CFG :
			{
				struct XMCS_GponTodCfg_S temp = {0};
				COPY_FROM_USER(&temp,(struct XMCS_GponTodCfg_S *)arg,sizeof(struct XMCS_GponTodCfg_S),ret);										
				ret = xmcs_set_gpon_tod(&temp) ;
			}
			break ;
		case GPON_IOG_CURRENT_TOD :
			{
				struct XMCS_GponTodCfg_S temp = {0};
				COPY_FROM_USER(&temp,(struct XMCS_GponTodCfg_S *)arg,sizeof(struct XMCS_GponTodCfg_S),ret);														
			    ret = xmcs_get_gpon_tod(&temp) ;			
				COPY_TO_USER((struct XMCS_GponTodCfg_S *)arg,&temp,sizeof(struct XMCS_GponTodCfg_S),ret);				
			}
			break ;
		case GPON_IOG_NEW_TOD :
			{
				struct XMCS_GponTodCfg_S temp = {0};
				COPY_FROM_USER(&temp,(struct XMCS_GponTodCfg_S *)arg,sizeof(struct XMCS_GponTodCfg_S),ret);	
			    ret = xmcs_get_gpon_new_tod(&temp) ;
				COPY_TO_USER((struct XMCS_GponTodCfg_S *)arg,&temp,sizeof(struct XMCS_GponTodCfg_S),ret);	
			}
			break ;
		case GPON_IOG_TOD_SWITCH_TIME :
			{
				struct XMCS_GponTodCfg_S temp = {0};
				COPY_FROM_USER(&temp,(struct XMCS_GponTodCfg_S *)arg,sizeof(struct XMCS_GponTodCfg_S),ret);	
			    ret = xmcs_get_tod_switch_time(&temp) ;
				COPY_TO_USER((struct XMCS_GponTodCfg_S *)arg,&temp,sizeof(struct XMCS_GponTodCfg_S),ret);
			}
			break ;
		case GPON_IOG_GET_GEM_COUNTER_CLEAR :
			{
				STATISTIC_CFG_t temp = {0};
				COPY_FROM_USER(&temp,(STATISTIC_CFG_t *)arg,sizeof(STATISTIC_CFG_t),ret);	
			    ret = xmcs_get_gem_counter_clear(&temp) ;
				COPY_TO_USER((STATISTIC_CFG_t *)arg,&temp,sizeof(STATISTIC_CFG_t),ret);
			}
			break ;
		case GPON_IOG_GEM_COUNTER :
			{
				STATISTIC_CFG_t temp = {0};
				COPY_FROM_USER(&temp,(STATISTIC_CFG_t *)arg,sizeof(STATISTIC_CFG_t),ret);	
			    ret = xmcs_get_gem_counter(&temp) ;
				COPY_TO_USER((STATISTIC_CFG_t *)arg,&temp,sizeof(STATISTIC_CFG_t),ret);
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
			{
				struct XMCS_EqdOffset_S temp = {0};
				COPY_FROM_USER(&temp,(struct XMCS_EqdOffset_S *)arg,sizeof(struct XMCS_EqdOffset_S),ret);					
			    ret = xmcs_get_eqd_offset(&temp) ;
				COPY_TO_USER((struct XMCS_EqdOffset_S *)arg,&temp,sizeof(struct XMCS_EqdOffset_S),ret);				
			}
			break ;
		case GPON_IOG_SUPER_FRAME_COUNTER :
			{
				uint temp = 0;
				ret = gponDevGetSuperframe(&temp);
				COPY_TO_USER((uint *)arg,&temp,sizeof(uint),ret);					
			}
			break ;
		case GPON_IOS_COUNTER_TYPE :
			ret = xmcs_set_counter_type((GPON_COUNTER_TYPE_t)arg) ;
			break ;
		case GPON_IOG_COUNTER_TYPE :
			{
				GPON_COUNTER_TYPE_t temp = GPON_COUNTER_TYPE_GEM;
				ret = xmcs_get_counter_type(&temp);
				COPY_TO_USER((GPON_COUNTER_TYPE_t *)arg,&temp,sizeof(GPON_COUNTER_TYPE_t),ret);				
			}
			break ;
		case GPON_IOS_RESPONSE_TIME :
			ret = xmcs_set_response_time((ushort)arg) ;
			break ;
		case GPON_IOG_RESPONSE_TIME :
			{
				ushort temp = 0;
				ret = xmcs_get_response_time(&temp);
				COPY_TO_USER((ushort *)arg,&temp,sizeof(ushort),ret);			
			}
			break ;
		case GPON_IOS_INTERNAL_DELAY_FINE_TUNE :
			ret = xmcs_set_internal_delay_fine_tune((unchar)arg) ;
			break ;
		case GPON_IOG_INTERNAL_DELAY_FINE_TUNE :
			{
				unchar temp = 0;
				ret = xmcs_get_internal_delay_fine_tune(&temp);
				COPY_TO_USER((unchar *)arg,&temp,sizeof(unchar),ret);	
			}
			break ;
		case GPON_IOS_BURST_MODE_OVERHEAD_LEN :
			ret = xmcs_set_burst_mode_overhead((GPON_BURST_MODE_OVERHEAD_LEN_T)arg) ;
			break ;
		case GPON_IOG_BURST_MODE_OVERHEAD_LEN :
			{
				GPON_BURST_MODE_OVERHEAD_LEN_T temp = GPON_BURST_MODE_OVERHEAD_LEN_DEFAULT;
				ret = xmcs_get_burst_mode_overhead(&temp);
				COPY_TO_USER((GPON_BURST_MODE_OVERHEAD_LEN_T *)arg,&temp,sizeof(GPON_BURST_MODE_OVERHEAD_LEN_T),ret);					
			}
			break ;
		case GPON_IOS_CLEAR_COUNTER :
			ret = xmcs_set_clear_counter((GPON_SW_HW_SELECT_T)arg) ;
			break ;
		case GPON_IOS_KEY_SWITCH_TIME :
			ret = xmcs_set_key_switch_time((uint)arg) ;
			break ;
		case GPON_IOS_ENCRYPT_KEY :
			{
				unchar temp[16] = {0};
				COPY_FROM_USER(temp,(unchar *)arg,sizeof(temp),ret);					
				ret = xmcs_set_encrypt_key(temp) ;
			}
			break ;
		case GPON_IOG_ENCRYPT_KEY_INFO :
			{
				GPON_DEV_ENCRYPT_KEY_INFO_T temp = {0};
				COPY_FROM_USER(&temp,(GPON_DEV_ENCRYPT_KEY_INFO_T *)arg,sizeof(GPON_DEV_ENCRYPT_KEY_INFO_T),ret);		
			    ret = xmcs_get_encrypt_key_info(&temp) ;
				COPY_TO_USER((GPON_DEV_ENCRYPT_KEY_INFO_T *)arg,&temp,sizeof(GPON_DEV_ENCRYPT_KEY_INFO_T),ret);					
			}
			break ;
		case GPON_IOS_DYING_GASP_MODE :
			ret = xmcs_set_dying_gasp_mode((GPON_SW_HW_SELECT_T)arg) ;
			break ;
		case GPON_IOG_DYING_GASP_MODE :
			{
				GPON_SW_HW_SELECT_T temp = GPON_SW;
				ret = xmcs_get_dying_gasp_mode(&temp);
				COPY_TO_USER((GPON_SW_HW_SELECT_T *)arg,&temp,sizeof(GPON_SW_HW_SELECT_T),ret);					
			}
			break ;
		case GPON_IOS_DYING_GASP_NUM :
			ret = xmcs_set_dying_gasp_num((uint)arg) ;
			break ;
		case GPON_IOG_DYING_GASP_NUM :
			{
				uint temp = 0;
				ret = xmcs_get_dying_gasp_num(&temp);
				COPY_TO_USER((uint *)arg,&temp,sizeof(uint),ret);		
			}
			break ;
#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7528)
		case GPON_IOS_DYING_GASP_INTVL :
			ret = xmcs_set_dying_gasp_intvl((uint)arg) ;
			break ;
		case GPON_IOG_DYING_GASP_INTVL :
			{
				uint temp = 0;
				ret = xmcs_get_dying_gasp_intvl(&temp);
				COPY_TO_USER((uint *)arg,&temp,sizeof(uint),ret);
			}
			break ;
#endif
		case GPON_IOS_UP_TRAFFIC :
			{
				GPON_DEV_UP_TRAFFIC_T temp = {0};
				COPY_FROM_USER(&temp,(GPON_DEV_UP_TRAFFIC_T *)arg,sizeof(GPON_DEV_UP_TRAFFIC_T),ret);	
				ret = xmcs_set_up_traffic(&temp) ;
			}
			break ;
		case GPON_IOG_UP_TRAFFIC :
			{
				GPON_DEV_UP_TRAFFIC_T temp = {0};
			    ret = xmcs_get_up_traffic(&temp) ;
				COPY_TO_USER((GPON_DEV_UP_TRAFFIC_T *)arg,&temp,sizeof(GPON_DEV_UP_TRAFFIC_T),ret);
			}
			break ;
		case GPON_IOG_INT_MASK :
			{
				uint temp = 0;
				ret = xmcs_get_int_mask(&temp);
				COPY_TO_USER((uint *)arg,&temp,sizeof(uint),ret);
			}
			break ;
#ifdef TCSUPPORT_CPU_EN7521
		case GPON_IOS_SNIFFER_GTC :
			{
				GPON_DEV_SNIFFER_MODE_T temp = {0};
				COPY_FROM_USER(&temp,(GPON_DEV_SNIFFER_MODE_T *)arg,sizeof(GPON_DEV_SNIFFER_MODE_T),ret);					
				ret = xmcs_set_sniffer_mode(&temp) ;
			}
			break ;
		case GPON_IOG_SNIFFER_GTC :
			{
				GPON_DEV_SNIFFER_MODE_T temp = {0};
				COPY_FROM_USER(&temp,(GPON_DEV_SNIFFER_MODE_T *)arg,sizeof(GPON_DEV_SNIFFER_MODE_T),ret);					
			    ret = xmcs_get_sniffer_mode(&temp) ;
				COPY_TO_USER((GPON_DEV_SNIFFER_MODE_T *)arg,&temp,sizeof(GPON_DEV_SNIFFER_MODE_T),ret);
			}
			break ;
		case GPON_IOS_EXT_BST_LEN_PLOAMD_FILTER_IN_O5 :
			ret = xmcs_set_ext_bst_len_ploamd_filter((XPON_Mode_t)arg) ;
			break ;
		case GPON_IOG_EXT_BST_LEN_PLOAMD_FILTER_IN_O5 :
			{
				XPON_Mode_t temp = XPON_DISABLE;
				ret = xmcs_get_ext_bst_len_ploamd_filter(&temp);
				COPY_TO_USER((XPON_Mode_t *)arg,&temp,sizeof(XPON_Mode_t),ret);				
			}
			break ;
		case GPON_IOS_UP_OVERHEAD_PLOAMD_FILTER_IN_O5 :
			ret = xmcs_set_up_overhead_ploamd_filter((XPON_Mode_t)arg) ;
			break ;
		case GPON_IOG_UP_OVERHEAD_PLOAMD_FILTER_IN_O5 :
			{
				XPON_Mode_t temp = XPON_DISABLE;
				ret = xmcs_get_up_overhead_ploamd_filter(&temp);
				COPY_TO_USER((XPON_Mode_t *)arg,&temp,sizeof(XPON_Mode_t),ret);					
			}			
			break ;
		case GPON_IOS_TX_4BYTES_ALIGN :
			ret = xmcs_set_tx_4bytes_align((XPON_Mode_t)arg) ;
			break ;
		case GPON_IOG_TX_4BYTES_ALIGN :
			{
				XPON_Mode_t temp = XPON_DISABLE;
				ret = xmcs_get_tx_4bytes_align(&temp);
				COPY_TO_USER((XPON_Mode_t *)arg,&temp,sizeof(XPON_Mode_t),ret);					
			}			
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
			{
				GPON_DEV_DBA_BACKDOOR_T temp = {0};
				COPY_FROM_USER(&temp,(GPON_DEV_DBA_BACKDOOR_T *)arg,sizeof(GPON_DEV_DBA_BACKDOOR_T),ret);
			    ret = xmcs_get_dba_backdoor(&temp) ;
				COPY_TO_USER((GPON_DEV_DBA_BACKDOOR_T *)arg,&temp,sizeof(GPON_DEV_DBA_BACKDOOR_T),ret);	
			}
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
			{
				GPON_DEV_SLIGHT_MODIFY_T temp = {0};
				COPY_FROM_USER(&temp,(GPON_DEV_SLIGHT_MODIFY_T *)arg,sizeof(GPON_DEV_SLIGHT_MODIFY_T),ret);
			    ret = xmcs_get_dba_slight_modify(&temp) ;
				COPY_TO_USER((GPON_DEV_SLIGHT_MODIFY_T *)arg,&temp,sizeof(GPON_DEV_SLIGHT_MODIFY_T),ret);	
			}
			break ;
		case GPON_IOS_DBA_SHIFT_MODIFY :
			ret = xmcs_set_dba_shift_modify((XPON_Mode_t)arg) ;
			break ;
		case GPON_IOS_DBA_SHIFT_MODIFY_TOTAL :
			{
				struct XMCS_DBAShiftMod_S temp = {0};
				COPY_FROM_USER(&temp,(struct XMCS_DBAShiftMod_S *)arg,sizeof(struct XMCS_DBAShiftMod_S),ret);				
				ret = xmcs_set_dba_shift_modify_total(&temp) ;
			}
			break ;
		case GPON_IOS_DBA_SHIFT_MODIFY_GREEN :
			{
				struct XMCS_DBAShiftMod_S temp = {0};
				COPY_FROM_USER(&temp,(struct XMCS_DBAShiftMod_S *)arg,sizeof(struct XMCS_DBAShiftMod_S),ret);
				ret = xmcs_set_dba_shift_modify_green(&temp) ;
			}			
			break ;
		case GPON_IOS_DBA_SHIFT_MODIFY_YELLOW :
			{
				struct XMCS_DBAShiftMod_S temp = {0};
				COPY_FROM_USER(&temp,(struct XMCS_DBAShiftMod_S *)arg,sizeof(struct XMCS_DBAShiftMod_S),ret);				
				ret = xmcs_set_dba_shift_modify_yellow(&temp) ;
			}			
			break ;
		case GPON_IOS_O3_O4_PLOAM_CTRL :
			ret = xmcs_set_o3_o4_ploam_ctrl((GPON_SW_HW_SELECT_T)arg) ;			
			break ;
		case GPON_IOG_O3_O4_PLOAM_CTRL :
			{
				GPON_SW_HW_SELECT_T temp = GPON_SW;
				ret = xmcs_get_o3_o4_ploam_ctrl(&temp);
				COPY_TO_USER((GPON_SW_HW_SELECT_T *)arg,&temp,sizeof(GPON_SW_HW_SELECT_T),ret);	
			}
			break ;
		case GPON_IOG_TX_SYNC :
			{
				unchar temp = 0;
				ret = xmcs_get_tx_sync_offset(&temp);
				COPY_TO_USER((unchar *)arg,&temp,sizeof(unchar),ret);	
			}
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
		case GPON_IOS_1PPS_HIGH_WIDTH :
			if (isEN7526c || isEN751627 || isEN7580 || isEN7523 || isEN7581 || isAN7583 || isAN7552) {
				ret = xmcs_set_1pps_h_w((uint)arg) ;
			} else {
				/* only supported by EN7526FC */
				ret = -1;
			}
			break ;
		case GPON_IOG_1PPS_HIGH_WIDTH :
			{
				if (isEN7526c || isEN751627 || isEN7580 ||isEN7523 || isEN7581 || isAN7583 || isAN7552) {
					uint temp = 0;
					ret = xmcs_get_1pps_h_w(&temp);
					COPY_TO_USER((uint *)arg,&temp,sizeof(uint),ret);						
				} else {
					/* only supported by EN7526FC */
					ret = -1;
				}
			}
			break ;
		case GPON_IOS_SEND_PLOAMU_WAIT_MODE :
			if (isEN7526c || isEN751627 || isEN7580 || isEN7523 || isEN7581 || isAN7583 || isAN7552) {
				ret = xmcs_set_send_ploamu_wait_mode((GPON_DEV_SEND_PLOAMU_WAIT_MODE_T)arg) ;
			} else {
				/* only supported by EN7526FC */
				ret = -1;
			}
			break ;
		case GPON_IOG_SEND_PLOAMU_WAIT_MODE :
			{
				if (isEN7526c || isEN751627 || isEN7580 || isEN7523 || isEN7581 || isAN7583 || isAN7552) {
					GPON_DEV_SEND_PLOAMU_WAIT_MODE_T temp = SEND_PLOAMU_BEFORE;
					ret = xmcs_get_send_ploamu_wait_mode(&temp);
					COPY_TO_USER((GPON_DEV_SEND_PLOAMU_WAIT_MODE_T *)arg,&temp,sizeof(GPON_DEV_SEND_PLOAMU_WAIT_MODE_T),ret);					
				} else {
					/* only supported by EN7526FC */
					ret = -1;
				}
			}
			break ;
#endif /* TCSUPPORT_CPU_EN7521 */
		case GPON_IOS_RESET_SERVICE :
			ret = xmcs_gpon_reset_service();
			break ;
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
			{
				struct XMCS_GponGetCounter_S temp;
				memset(&temp, 0, sizeof(struct XMCS_GponGetCounter_S)) ;
				COPY_FROM_USER(&temp,(struct XMCS_GponGetCounter_S *)arg,sizeof(struct XMCS_GponGetCounter_S),ret);					
			    ret = xmcs_get_rx_ethernet_frame_counter(&temp) ;
				COPY_TO_USER((struct XMCS_GponGetCounter_S *)arg,&temp,sizeof(struct XMCS_GponGetCounter_S),ret);				
			}			
			break;
		case GPON_GET_TRAFFIC_STATE :
			{
				uint temp = 0;
				ret = xmcs_get_gpon_traffic_state(&temp);
				COPY_TO_USER((uint *)arg,&temp,sizeof(uint),ret);					
			}			
			break;
		case GPON_IOS_TEST_HOTPLUG :
			ret = xmcs_test_gpon_hotplug((XPON_Mode_t) arg);
			break;
#ifndef TCSUPPORT_CPU_EN7521
		case XPON_GET_UPSTREAM_BLOCK_STATE :
			{
				uint temp = 0;
				ret = xmcs_get_upstream_block_state(&temp);
				COPY_TO_USER((uint *)arg,&temp,sizeof(uint),ret);				
			}			
			break;
#endif /* TCSUPPORT_CPU_EN7521 */
            
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HYBIRD)
        case GPON_ADD_HYBIRD_ROUTE_MAC :
			{
				GPON_HYBIRD_ROUTE_MAC_T temp = {0};
				COPY_FROM_USER(&temp,(GPON_HYBIRD_ROUTE_MAC_T *)arg,sizeof(GPON_HYBIRD_ROUTE_MAC_T),ret);					
				ret = xmcs_add_hybird_route_mac(&temp) ;
			}
            break;
        case GPON_DEL_HYBIRD_ROUTE_MAC :
			{
				GPON_HYBIRD_ROUTE_MAC_T temp = {0};
				COPY_FROM_USER(&temp,(GPON_HYBIRD_ROUTE_MAC_T *)arg,sizeof(GPON_HYBIRD_ROUTE_MAC_T),ret);					
				ret = xmcs_del_hybird_route_mac(&temp) ;
			}			
            break;
#endif/*TCSUPPORT_COMPILE*/
		case GPON_IOS_MIB_FLAG :
            ret = xmcs_set_mib_flag((uint)arg) ;
            break;
        case GPON_IOS_SD_SF_THRESHOLD :
			{
				GPON_DEV_SD_SF_THLD_T temp = {0};
				COPY_FROM_USER(&temp,(GPON_DEV_SD_SF_THLD_T *)arg,sizeof(GPON_DEV_SD_SF_THLD_T),ret);
				ret = xmcs_set_SD_SF_threshold(&temp) ;
			}
			break ;	
        case GPON_IOG_SD_SF_THRESHOLD :
			{
				GPON_DEV_SD_SF_THLD_T temp = {0};
				COPY_FROM_USER(&temp,(GPON_DEV_SD_SF_THLD_T *)arg,sizeof(GPON_DEV_SD_SF_THLD_T),ret);
				ret = xmcs_get_SD_SF_threshold(&temp) ;
				COPY_TO_USER((GPON_DEV_SD_SF_THLD_T *)arg,&temp,sizeof(GPON_DEV_SD_SF_THLD_T),ret);
			}
			break ;	
        case GPON_IOG_SD_SF_CNT :
			{
				GPON_DEV_SD_SF_CNT_T temp = {0};
				COPY_FROM_USER(&temp,(GPON_DEV_SD_SF_CNT_T *)arg,sizeof(GPON_DEV_SD_SF_CNT_T),ret);
				ret = xmcs_get_SD_SF_cnt(&temp) ;
				COPY_TO_USER((GPON_DEV_SD_SF_CNT_T *)arg,&temp,sizeof(GPON_DEV_SD_SF_CNT_T),ret);
			}
			break ;
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
				COPY_TO_USER((unchar *)arg,&gGpon_SD_SF_Info.SF_thld,sizeof(unchar),ret);
			}
			break;
		case RDKB_IOS_SIGNAL_DEGRADE_THRESHOLD:
			ret = 0;
			gpPhyData->signal_degrade = (uint)arg;
			break;
		case RDKB_IOG_SIGNAL_DEGRADE_THRESHOLD:
			{
				ret = 0;
				COPY_TO_USER((unchar *)arg,&gGpon_SD_SF_Info.SD_thld,sizeof(unchar),ret);
			}
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

