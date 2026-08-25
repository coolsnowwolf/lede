/*
* File Name: xmcs_if.c
* Description: WAN Interface I/O Command Process for xPON 
*              Management Control Subsystem
*
******************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/sched.h>

#include "common/drv_global.h"
#include "xmcs/xmcs_gpon.h"
#include "xmcs/xmcs_if.h"

#ifdef TCSUPPORT_CPU_EN7521
#include <ecnt_hook/ecnt_hook_fe.h>
#else
#include "../../../raeth/fe_api.h"
#include "epon/fe_reg.h"
#endif
#include "xmcs/xmcs_phy.h"
#include "../../inc/common/phy_if_wrapper.h"

#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
extern int (*sw_qosCreatPolicer) (int id, int bandwidth);
extern int (*sw_qosDeletePolicer) (int id);
extern int (*sw_qosOverallRatelimit) (int bandwidth);
extern int gQueueMask;
#define		QOS_HW_CAR			5
#define     TX_QUEUE_NUM        8
#define     CAR_QUEUE_NUM       6
extern int trtcmEnable;
int isTotalBWSet = 0;
#endif

#ifdef TCSUPPORT_QOS
extern char qosFlag;
#endif


extern int isNeedResetTransciver;
/***************************************************************
***************************************************************/
static int xmcs_null(uint null) 
{
	printk("This function doesn't implement yet!\n") ;	
	return 0 ;
}

/***************************************************************
typedef enum {
	XMCS_IF_WEIGHT_TYPE_PACKET = 0,
	XMCS_IF_WEIGHT_TYPE_BYTE
} XMCSIF_QoSWeightType_t ;

typedef enum {
	XMCS_IF_WEIGHT_SCALE_64B = 0,
	XMCS_IF_WEIGHT_SCALE_16B
} XMCSIF_QoSWeightScale_t ;

struct XMCS_QoSWeightConfig_S {
	XMCSIF_QoSWeightType_t		weightType ;
	XMCSIF_QoSWeightScale_t		weightScale ;
} ;
***************************************************************/
static int xmcs_set_qos_weight_config(struct XMCS_QoSWeightConfig_S *pWeightCfg) 
{
	QDMA_TxQosWeightType_t qdmaWeightType ;
	QDMA_TxQosWeightScale_t qdmaWeightScale ;
	
	if(pWeightCfg->weightType == XMCS_IF_WEIGHT_TYPE_PACKET) {
		qdmaWeightType = QDMA_TXQOS_WEIGHT_BY_PACKET ;
	} else if(pWeightCfg->weightType == XMCS_IF_WEIGHT_TYPE_BYTE) {
		qdmaWeightType = QDMA_TXQOS_WEIGHT_BY_BYTE ;
	} else {
		return -EINVAL ;
	}
	
	if(pWeightCfg->weightType == XMCS_IF_WEIGHT_TYPE_BYTE) {
		if(pWeightCfg->weightScale == XMCS_IF_WEIGHT_SCALE_64B) {
			qdmaWeightScale = QDMA_TXQOS_WEIGHT_SCALE_64B ;
		} else if(pWeightCfg->weightScale == XMCS_IF_WEIGHT_SCALE_16B) {
			qdmaWeightScale = QDMA_TXQOS_WEIGHT_SCALE_16B ;
		} else {
			return -EINVAL ;
		}
	}
#ifdef TCSUPPORT_CPU_EN7521
	return QDMA_API_SET_TX_QOS_WEIGHT(ECNT_QDMA_WAN, qdmaWeightType, qdmaWeightScale) ;
#else /* TCSUPPORT_CPU_EN7521 */
	return qdma_set_tx_qos_weight(qdmaWeightType, qdmaWeightScale) ;
#endif /* TCSUPPORT_CPU_EN7521 */
}

/***************************************************************
typedef enum {
	XMCS_IF_WEIGHT_TYPE_PACKET = 0,
	XMCS_IF_WEIGHT_TYPE_BYTE
} XMCSIF_QoSWeightType_t ;

typedef enum {
	XMCS_IF_WEIGHT_SCALE_64B = 0,
	XMCS_IF_WEIGHT_SCALE_16B
} XMCSIF_QoSWeightScale_t ;

struct XMCS_QoSWeightConfig_S {
	XMCSIF_QoSWeightType_t		weightType ;
	XMCSIF_QoSWeightScale_t		weightScale ;
} ;
***************************************************************/
static int xmcs_get_qos_weight_config(struct XMCS_QoSWeightConfig_S *pWeightCfg) 
{
	int ret = 0 ;
	QDMA_TxQosWeightType_t qdmaWeightType ;
	QDMA_TxQosWeightScale_t qdmaWeightScale ;

#ifdef TCSUPPORT_CPU_EN7521
	ret = QDMA_API_GET_TX_QOS_WEIGHT(ECNT_QDMA_WAN, &qdmaWeightType, &qdmaWeightScale) ;
#else /* TCSUPPORT_CPU_EN7521 */
	ret = qdma_get_tx_qos_weight(&qdmaWeightType, &qdmaWeightScale) ;
#endif /* TCSUPPORT_CPU_EN7521 */

	if(ret != 0) {
		return ret ;
	}

	pWeightCfg->weightType = (qdmaWeightType==QDMA_TXQOS_WEIGHT_BY_PACKET) ? XMCS_IF_WEIGHT_TYPE_PACKET : XMCS_IF_WEIGHT_TYPE_BYTE ;
	pWeightCfg->weightScale = (qdmaWeightScale==QDMA_TXQOS_WEIGHT_SCALE_64B) ? XMCS_IF_WEIGHT_SCALE_64B : XMCS_IF_WEIGHT_SCALE_16B ;
		
	return 0 ;
}

/***************************************************************
 struct XMCS_ChannelQoS_S {
 	unchar						channel ;
 	XMCSIF_QosType_t			qosType ;
 	struct {
 		unchar					weight ;
 	} queue[XPON_QUEUE_NUMBER] ;
 } ;
***************************************************************/
int xmcs_set_channel_scheduler(struct XMCS_ChannelQoS_S *pScheduler)
{
	QDMA_TxQosScheduler_T qosScheduler ;
	int i ;
	
	if(pScheduler->channel<0 || pScheduler->channel>=XPON_CHANNEL_NUMBER) {
		return -EINVAL ;
	}
	qosScheduler.channel = pScheduler->channel ;
	
	if(pScheduler->qosType == XMCS_IF_QOS_TYPE_WRR) {
		qosScheduler.qosType = QDMA_TXQOS_TYPE_WRR ;
	} else if(pScheduler->qosType == XMCS_IF_QOS_TYPE_SP) {
		qosScheduler.qosType = QDMA_TXQOS_TYPE_SP ;
	} else if(pScheduler->qosType == XMCS_IF_QOS_TYPE_SPWRR7) {
		qosScheduler.qosType = QDMA_TXQOS_TYPE_SPWRR7 ;
	} else if(pScheduler->qosType == XMCS_IF_QOS_TYPE_SPWRR6) {
		qosScheduler.qosType = QDMA_TXQOS_TYPE_SPWRR6 ;
	} else if(pScheduler->qosType == XMCS_IF_QOS_TYPE_SPWRR5) {
		qosScheduler.qosType = QDMA_TXQOS_TYPE_SPWRR5 ;
	} else if(pScheduler->qosType == XMCS_IF_QOS_TYPE_SPWRR4) {
		qosScheduler.qosType = QDMA_TXQOS_TYPE_SPWRR4 ;
	} else if(pScheduler->qosType == XMCS_IF_QOS_TYPE_SPWRR3) {
		qosScheduler.qosType = QDMA_TXQOS_TYPE_SPWRR3 ;
	} else if(pScheduler->qosType == XMCS_IF_QOS_TYPE_SPWRR2) {
		qosScheduler.qosType = QDMA_TXQOS_TYPE_SPWRR2 ;
	} else {
		return -EINVAL ;
	}
		
	for(i=0 ; i<CONFIG_QDMA_QUEUE ; i++) {
		qosScheduler.queue[i].weight = pScheduler->queue[i].weight ;
	}
	
#ifdef TCSUPPORT_CPU_EN7521
	return QDMA_API_SET_TX_QOS(ECNT_QDMA_WAN, &qosScheduler);
#else /* TCSUPPORT_CPU_EN7521 */
	return qdma_set_tx_qos(&qosScheduler);
#endif /* TCSUPPORT_CPU_EN7521 */
}
EXPORT_SYMBOL(xmcs_set_channel_scheduler);

/***************************************************************
 struct XMCS_ChannelQoS_S {
 	unchar						channel ;
 	XMCSIF_QosType_t			qosType ;
 	struct {
 		unchar					weight ;
 	} queue[XPON_QUEUE_NUMBER] ;
 } ;
***************************************************************/
static int xmcs_get_channel_scheduler(struct XMCS_ChannelQoS_S *pScheduler)
{
	QDMA_TxQosScheduler_T qosScheduler ;
	int i, ret=0 ;
	
	if(pScheduler->channel<0 || pScheduler->channel>=CONFIG_GPON_MAX_TCONT) {
		return -EINVAL ;
	}
	qosScheduler.channel = pScheduler->channel ;
	
#ifdef TCSUPPORT_CPU_EN7521
	ret = QDMA_API_GET_TX_QOS(ECNT_QDMA_WAN, &qosScheduler);
#else /* TCSUPPORT_CPU_EN7521 */
	ret = qdma_get_tx_qos(&qosScheduler);
#endif /* TCSUPPORT_CPU_EN7521 */
	if(ret != 0) {
		return ret ;
	}
	
	if(qosScheduler.qosType == QDMA_TXQOS_TYPE_WRR) {
		pScheduler->qosType = XMCS_IF_QOS_TYPE_WRR ;
	} else if(qosScheduler.qosType == QDMA_TXQOS_TYPE_SP) {
		pScheduler->qosType = XMCS_IF_QOS_TYPE_SP ;
	} else if(qosScheduler.qosType == QDMA_TXQOS_TYPE_SPWRR7) {
		pScheduler->qosType = XMCS_IF_QOS_TYPE_SPWRR7 ;
	} else if(qosScheduler.qosType == QDMA_TXQOS_TYPE_SPWRR6) {
		pScheduler->qosType = XMCS_IF_QOS_TYPE_SPWRR6 ;
	} else if(qosScheduler.qosType == QDMA_TXQOS_TYPE_SPWRR5) {
		pScheduler->qosType = XMCS_IF_QOS_TYPE_SPWRR5 ;
	} else if(qosScheduler.qosType == QDMA_TXQOS_TYPE_SPWRR4) {
		pScheduler->qosType = XMCS_IF_QOS_TYPE_SPWRR4 ;
	} else if(qosScheduler.qosType == QDMA_TXQOS_TYPE_SPWRR3) {
		pScheduler->qosType = XMCS_IF_QOS_TYPE_SPWRR3 ;
	} else if(qosScheduler.qosType == QDMA_TXQOS_TYPE_SPWRR2) {
		pScheduler->qosType = XMCS_IF_QOS_TYPE_SPWRR2 ;
	} 
	
	for(i=0 ; i<XPON_QUEUE_NUMBER ; i++) {
		pScheduler->queue[i].weight = qosScheduler.queue[i].weight ;
	}
	
	return ret ;
}

/***************************************************************
struct XMCS_TxQueueCongestion_S {
	struct {
		XPON_Mode_t				trtcm ;
		XPON_Mode_t				dei ;
		XPON_Mode_t				threshold ;
	} congestMode ;
	struct {
		XMCSIF_CongestionScale_t	max ;
		XMCSIF_CongestionScale_t	min ;
	} scale ;
	struct {
		unchar					green ;
		unchar					yellow ;
	} dropProbability ;
	struct {
		unchar					queueIdx ;
		unchar					greenMax ;
		unchar					greenMin ;
		unchar					yellowMax ;
		unchar					yellowMin ;
	} queueThreshold[XPON_QUEUE_NUMBER] ;
} ;
***************************************************************/
static int xmcs_set_trtcm_congest_mode(struct XMCS_TxQueueCongestion_S *pCongest)
{
	QDMA_Mode_t mode ;
	
	mode = (pCongest->congestMode.trtcm==XPON_ENABLE) ? QDMA_ENABLE : QDMA_DISABLE ;	
#ifdef TCSUPPORT_CPU_EN7521
	//todo: not implement yet
#else
	qdma_set_txqueue_trtcm_mode(mode) ;
#endif
	return 0 ;
}

/***************************************************************
struct XMCS_TxQueueCongestion_S {
	struct {
		XPON_Mode_t				trtcm ;
		XPON_Mode_t				dei ;
		XPON_Mode_t				threshold ;
	} congestMode ;
	struct {
		XMCSIF_CongestionScale_t	max ;
		XMCSIF_CongestionScale_t	min ;
	} scale ;
	struct {
		unchar					green ;
		unchar					yellow ;
	} dropProbability ;
	struct {
		unchar					queueIdx ;
		unchar					greenMax ;
		unchar					greenMin ;
		unchar					yellowMax ;
		unchar					yellowMin ;
	} queueThreshold[XPON_QUEUE_NUMBER] ;
} ;
***************************************************************/
static int xmcs_set_dei_congest_mode(struct XMCS_TxQueueCongestion_S *pCongest)
{
	QDMA_Mode_t mode ;
	
	mode = (pCongest->congestMode.dei==XPON_ENABLE) ? QDMA_ENABLE : QDMA_DISABLE ;	
#ifdef TCSUPPORT_CPU_EN7521
	QDMA_API_SET_TXQ_DEI_DROP_MODE(ECNT_QDMA_WAN, mode);
#else
	qdma_set_txqueue_dei_mode(mode) ;
#endif
	
	return 0 ;
}

/***************************************************************
struct XMCS_TxQueueCongestion_S {
	struct {
		XPON_Mode_t				trtcm ;
		XPON_Mode_t				dei ;
		XPON_Mode_t				threshold ;
	} congestMode ;
	struct {
		XMCSIF_CongestionScale_t	max ;
		XMCSIF_CongestionScale_t	min ;
	} scale ;
	struct {
		unchar					green ;
		unchar					yellow ;
	} dropProbability ;
	struct {
		unchar					queueIdx ;
		unchar					greenMax ;
		unchar					greenMin ;
		unchar					yellowMax ;
		unchar					yellowMin ;
	} queueThreshold[XPON_QUEUE_NUMBER] ;
} ;
***************************************************************/
static int xmcs_set_threshold_congest_mode(struct XMCS_TxQueueCongestion_S *pCongest)
{
	QDMA_Mode_t mode ;

	if (pCongest->congestMode.threshold == XPON_OTHER){
		xpon_set_qdma_qos(gpWanPriv->devCfg.flags.isQosUp);
	} else {
		mode = (pCongest->congestMode.threshold==XPON_ENABLE) ? QDMA_ENABLE : QDMA_DISABLE ;	
#ifdef TCSUPPORT_CPU_EN7521
		//todo: not implement yet
#else
		qdma_set_txqueue_threshold_mode(mode) ;
#endif
	}
	
	return 0 ;
}

/***************************************************************
struct XMCS_TxQueueCongestion_S {
	struct {
		XPON_Mode_t				trtcm ;
		XPON_Mode_t				dei ;
		XPON_Mode_t				threshold ;
	} congestMode ;
	struct {
		XMCSIF_CongestionScale_t	max ;
		XMCSIF_CongestionScale_t	min ;
	} scale ;
	struct {
		unchar					green ;
		unchar					yellow ;
	} dropProbability ;
	struct {
		unchar					queueIdx ;
		unchar					greenMax ;
		unchar					greenMin ;
		unchar					yellowMax ;
		unchar					yellowMin ;
	} queueThreshold[XPON_QUEUE_NUMBER] ;
} ;
typedef enum {
	XMCS_IF_CONGESTIOM_SCALE_2 = 0,
	XMCS_IF_CONGESTIOM_SCALE_4,
	XMCS_IF_CONGESTIOM_SCALE_8,
	XMCS_IF_CONGESTIOM_SCALE_16,
} XMCSIF_CongestionScale_t ;
***************************************************************/
static int xmcs_set_congest_scale(struct XMCS_TxQueueCongestion_S *pCongest)
{
	QDMA_TxQueueCongestScale_T setScale ;
	
	if(pCongest->scale.max == XMCS_IF_CONGESTIOM_SCALE_2) {
		setScale.maxScale = QDMA_TXQUEUE_SCALE_2_DSCP ;
	} else if(pCongest->scale.max == XMCS_IF_CONGESTIOM_SCALE_4) {
		setScale.maxScale = QDMA_TXQUEUE_SCALE_4_DSCP ;
	} else if(pCongest->scale.max == XMCS_IF_CONGESTIOM_SCALE_8) {
		setScale.maxScale = QDMA_TXQUEUE_SCALE_8_DSCP ;
	} else if(pCongest->scale.max == XMCS_IF_CONGESTIOM_SCALE_16) {
		setScale.maxScale = QDMA_TXQUEUE_SCALE_16_DSCP ;
	} else {
		return -EINVAL ;
	}
	
	if(pCongest->scale.min == XMCS_IF_CONGESTIOM_SCALE_2) {
		setScale.minScale = QDMA_TXQUEUE_SCALE_2_DSCP ;
	} else if(pCongest->scale.min == XMCS_IF_CONGESTIOM_SCALE_4) {
		setScale.minScale = QDMA_TXQUEUE_SCALE_4_DSCP ;
	} else if(pCongest->scale.min == XMCS_IF_CONGESTIOM_SCALE_8) {
		setScale.minScale = QDMA_TXQUEUE_SCALE_8_DSCP ;
	} else if(pCongest->scale.min == XMCS_IF_CONGESTIOM_SCALE_16) {
		setScale.minScale = QDMA_TXQUEUE_SCALE_16_DSCP ;
	} else {
		return -EINVAL ;
	}
#ifdef TCSUPPORT_CPU_EN7521
	//todo: not implement yet
	return 0;
#else
	return qdma_set_congestion_scale(&setScale) ;
#endif
}

#ifndef TCSUPPORT_CPU_EN7521
/***************************************************************
struct XMCS_TxQueueCongestion_S {
	struct {
		XPON_Mode_t				trtcm ;
		XPON_Mode_t				dei ;
		XPON_Mode_t				threshold ;
	} congestMode ;
	struct {
		XMCSIF_CongestionScale_t	max ;
		XMCSIF_CongestionScale_t	min ;
	} scale ;
	struct {
		unchar					green ;
		unchar					yellow ;
	} dropProbability ;
	struct {
		unchar					queueIdx ;
		unchar					greenMax ;
		unchar					greenMin ;
		unchar					yellowMax ;
		unchar					yellowMin ;
	} queueThreshold[XPON_QUEUE_NUMBER] ;
} ;
***************************************************************/
static int xmcs_set_congest_drop_probability(struct XMCS_TxQueueCongestion_S *pCongest)
{
	QDMA_TxQueueDropProbability_T setProbability ;
	
	if((pCongest->dropProbability.green<0 || pCongest->dropProbability.green>255) || 
	   (pCongest->dropProbability.yellow<0 || pCongest->dropProbability.yellow>255)) {
		return -EINVAL ;
	}

	setProbability.green = pCongest->dropProbability.green ;
	setProbability.yellow = pCongest->dropProbability.yellow ;
#ifndef TCSUPPORT_CPU_EN7521
	return qdma_set_congest_drop_probability(&setProbability) ;
#else
	return 0;
#endif
}
#endif

/***************************************************************
struct XMCS_TxQueueCongestion_S {
	struct {
		XPON_Mode_t				trtcm ;
		XPON_Mode_t				dei ;
		XPON_Mode_t				threshold ;
	} congestMode ;
	struct {
		XMCSIF_CongestionScale_t	max ;
		XMCSIF_CongestionScale_t	min ;
	} scale ;
	struct {
		unchar					green ;
		unchar					yellow ;
	} dropProbability ;
	struct {
		unchar					queueIdx ;
		unchar					greenMax ;
		unchar					greenMin ;
		unchar					yellowMax ;
		unchar					yellowMin ;
	} queueThreshold[XPON_QUEUE_NUMBER] ;
} ;
***************************************************************/
static int xmcs_set_congest_threshold(struct XMCS_TxQueueCongestion_S *pCongest)
{
#ifdef TCSUPPORT_CPU_EN7521
	//todo: not implement yet
	return 0;
#else
	QDMA_TxQueueCongestThreshold_T setThreshold ;
	
	if(pCongest->queueThreshold[0].queueIdx<0 || pCongest->queueThreshold[0].queueIdx>=XPON_QUEUE_NUMBER) {
	   return -EINVAL ;
	}
	
	setThreshold.queueIdx = pCongest->queueThreshold[0].queueIdx ;
	setThreshold.grnMaxThreshold = pCongest->queueThreshold[0].greenMax ;
	setThreshold.grnMinThreshold = pCongest->queueThreshold[0].greenMin ;
	setThreshold.ylwMaxThreshold = pCongest->queueThreshold[0].yellowMax ;
	setThreshold.ylwMinThreshold = pCongest->queueThreshold[0].yellowMin ;
	
	return qdma_set_congest_threshold(&setThreshold) ;
#endif
}

/***************************************************************
struct XMCS_TxQueueCongestion_S {
	struct {
		XPON_Mode_t				trtcm ;
		XPON_Mode_t				dei ;
		XPON_Mode_t				threshold ;
	} congestMode ;
	struct {
		XMCSIF_CongestionScale_t	max ;
		XMCSIF_CongestionScale_t	min ;
	} scale ;
	struct {
		unchar					green ;
		unchar					yellow ;
	} dropProbability ;
	struct {
		unchar					queueIdx ;
		unchar					greenMax ;
		unchar					greenMin ;
		unchar					yellowMax ;
		unchar					yellowMin ;
	} queueThreshold[XPON_QUEUE_NUMBER] ;
} ;
***************************************************************/
static int xmcs_get_congest_info(struct XMCS_TxQueueCongestion_S *pCongest)
{
	QDMA_TxQueueCongestCfg_T getCongest ;
	int i, ret = 0 ;
	
#ifdef TCSUPPORT_CPU_EN7521
	//todo: not implement yet
#else
	ret = qdma_get_congest_config(&getCongest) ;
	if(ret != 0) {
		return ret ;	
	}
	
	pCongest->congestMode.trtcm = (qdma_get_txqueue_trtcm_mode()==QDMA_ENABLE) ? XPON_ENABLE : XPON_DISABLE ;
	pCongest->congestMode.dei = (qdma_get_txqueue_dei_mode()==QDMA_ENABLE) ? XPON_ENABLE : XPON_DISABLE ;
	pCongest->congestMode.threshold = (qdma_get_txqueue_threshold_mode()==QDMA_ENABLE) ? XPON_ENABLE : XPON_DISABLE ;

	if(getCongest.maxScale == QDMA_TXQUEUE_SCALE_2_DSCP) {
		pCongest->scale.max = XMCS_IF_CONGESTIOM_SCALE_2 ;
	} else if(getCongest.maxScale == QDMA_TXQUEUE_SCALE_4_DSCP) {
		pCongest->scale.max = XMCS_IF_CONGESTIOM_SCALE_4 ;
	} else if(getCongest.maxScale == QDMA_TXQUEUE_SCALE_8_DSCP) {
		pCongest->scale.max = XMCS_IF_CONGESTIOM_SCALE_8 ;
	} else if(getCongest.maxScale == QDMA_TXQUEUE_SCALE_16_DSCP) {
		pCongest->scale.max = XMCS_IF_CONGESTIOM_SCALE_16 ;
	}

	if(getCongest.minScale == QDMA_TXQUEUE_SCALE_2_DSCP) {
		pCongest->scale.min = XMCS_IF_CONGESTIOM_SCALE_2 ;
	} else if(getCongest.minScale == QDMA_TXQUEUE_SCALE_4_DSCP) {
		pCongest->scale.min = XMCS_IF_CONGESTIOM_SCALE_4 ;
	} else if(getCongest.minScale == QDMA_TXQUEUE_SCALE_8_DSCP) {
		pCongest->scale.min = XMCS_IF_CONGESTIOM_SCALE_8 ;
	} else if(getCongest.minScale == QDMA_TXQUEUE_SCALE_16_DSCP) {
		pCongest->scale.min = XMCS_IF_CONGESTIOM_SCALE_16 ;
	}
	
	pCongest->dropProbability.green = getCongest.grnDropProb ;
	pCongest->dropProbability.yellow = getCongest.ylwDropProb ;
	for(i=0 ; i<XPON_QUEUE_NUMBER ; i++) {
		pCongest->queueThreshold[i].queueIdx = i ;
		pCongest->queueThreshold[i].greenMax = getCongest.queue[i].grnMaxThreshold ;
		pCongest->queueThreshold[i].greenMin = getCongest.queue[i].grnMinThreshold ;
		pCongest->queueThreshold[i].yellowMax = getCongest.queue[i].ylwMaxThreshold ;
		pCongest->queueThreshold[i].yellowMin = getCongest.queue[i].ylwMinThreshold ;
	}
#endif
	
	return ret ;
}

/***************************************************************
typedef enum {
	XMCS_IF_TRTCM_SCALE_1B = 0,
	XMCS_IF_TRTCM_SCALE_2B,
	XMCS_IF_TRTCM_SCALE_4B, 
	XMCS_IF_TRTCM_SCALE_8B,
	XMCS_IF_TRTCM_SCALE_16B,
	XMCS_IF_TRTCM_SCALE_32B,
	XMCS_IF_TRTCM_SCALE_64B,
	XMCS_IF_TRTCM_SCALE_128B,
	XMCS_IF_TRTCM_SCALE_256B,
	XMCS_IF_TRTCM_SCALE_512B,
	XMCS_IF_TRTCM_SCALE_1K,
	XMCS_IF_TRTCM_SCALE_2K,
	XMCS_IF_TRTCM_SCALE_4K,
	XMCS_IF_TRTCM_SCALE_8K,
	XMCS_IF_TRTCM_SCALE_16K,
	XMCS_IF_TRTCM_SCALE_32K,
} XMCSIF_TrtcmScale_t ;
struct XMCS_TxTrtcmScale_S {
	XMCSIF_TrtcmScale_t			trtcmScale ;
} ;
***************************************************************/
static int xmcs_set_tx_trtcm_scale(struct XMCS_TxTrtcmScale_S *pScale)
{
	QDMA_TrtcmScale_t qdmaTrtcmScale[XMCS_IF_TRTCM_SCALE_ITEMS] = {	QDMA_TRTCM_SCALE_1BYTE,
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

	if(pScale->trtcmScale<0 || pScale->trtcmScale>=XMCS_IF_TRTCM_SCALE_ITEMS) {
		return -EINVAL ;
	}
#ifdef TCSUPPORT_CPU_EN7521
		//todo: not implement yet
#else
	qdma_set_txqueue_trtcm_scale(qdmaTrtcmScale[pScale->trtcmScale]) ;
#endif
	return 0 ;
}

/***************************************************************
typedef enum {
	XMCS_IF_TRTCM_SCALE_1B = 0,
	XMCS_IF_TRTCM_SCALE_2B,
	XMCS_IF_TRTCM_SCALE_4B, 
	XMCS_IF_TRTCM_SCALE_8B,
	XMCS_IF_TRTCM_SCALE_16B,
	XMCS_IF_TRTCM_SCALE_32B,
	XMCS_IF_TRTCM_SCALE_64B,
	XMCS_IF_TRTCM_SCALE_128B,
	XMCS_IF_TRTCM_SCALE_256B,
	XMCS_IF_TRTCM_SCALE_512B,
	XMCS_IF_TRTCM_SCALE_1K,
	XMCS_IF_TRTCM_SCALE_2K,
	XMCS_IF_TRTCM_SCALE_4K,
	XMCS_IF_TRTCM_SCALE_8K,
	XMCS_IF_TRTCM_SCALE_16K,
	XMCS_IF_TRTCM_SCALE_32K,
} XMCSIF_TrtcmScale_t ;
struct XMCS_TxTrtcmScale_S {
	XMCSIF_TrtcmScale_t			trtcmScale ;
} ;
***************************************************************/
static int xmcs_get_tx_trtcm_scale(struct XMCS_TxTrtcmScale_S *pScale)
{
	QDMA_TrtcmScale_t qdmaTrtcmScale ;
	XMCSIF_TrtcmScale_t xmcsTrtcmScale[QDMA_TRTCM_SCALE_MAX_ITEMS] = { XMCS_IF_TRTCM_SCALE_1B,
																	   XMCS_IF_TRTCM_SCALE_2B,
																	   XMCS_IF_TRTCM_SCALE_4B, 
																	   XMCS_IF_TRTCM_SCALE_8B,
																	   XMCS_IF_TRTCM_SCALE_16B,
																	   XMCS_IF_TRTCM_SCALE_32B,
																	   XMCS_IF_TRTCM_SCALE_64B,
																	   XMCS_IF_TRTCM_SCALE_128B,
																	   XMCS_IF_TRTCM_SCALE_256B,
																	   XMCS_IF_TRTCM_SCALE_512B,
																	   XMCS_IF_TRTCM_SCALE_1K,
																	   XMCS_IF_TRTCM_SCALE_2K,
																	   XMCS_IF_TRTCM_SCALE_4K,
																	   XMCS_IF_TRTCM_SCALE_8K,
																	   XMCS_IF_TRTCM_SCALE_16K,
																	   XMCS_IF_TRTCM_SCALE_32K } ;
#ifndef TCSUPPORT_CPU_EN7521

	qdmaTrtcmScale = qdma_get_txqueue_trtcm_scale() ;
#endif
	if(qdmaTrtcmScale >= QDMA_TRTCM_SCALE_MAX_ITEMS) {
		return -EFAULT ;
	}
	
	pScale->trtcmScale = xmcsTrtcmScale[qdmaTrtcmScale] ;
	
	return 0 ;
}

/***************************************************************
struct XMCS_TxQueueTrtcm_S {
	unchar					tsIdx ;
	ushort					cirValue ;
	ushort					cbsUnit ;
	ushort					pirValue ;
	ushort					pbsUnit ;
} ;
***************************************************************/
static int xmcs_set_tx_trtcm_params(struct XMCS_TxQueueTrtcm_S *pTrtcmParams)
{
	QDMA_TxQueueTrtcm_T qdmaTrtcmParams ;
	
	if(pTrtcmParams->tsIdx<0 || pTrtcmParams->tsIdx>=XPON_SHAPING_NUMBER) {
		return -EINVAL ;
	}
	
	qdmaTrtcmParams.tsid = pTrtcmParams->tsIdx ;
	qdmaTrtcmParams.cirParamValue = pTrtcmParams->cirValue ;
	qdmaTrtcmParams.cbsParamValue = pTrtcmParams->cbsUnit ;
	qdmaTrtcmParams.pirParamValue = pTrtcmParams->pirValue ;
	qdmaTrtcmParams.pbsParamValue = pTrtcmParams->pbsUnit ;
#ifdef TCSUPPORT_CPU_EN7521
	//todo: not implement yet
	return 0;
#else
	return qdma_set_txqueue_trtcm_params(&qdmaTrtcmParams) ;
#endif
}

/***************************************************************
struct XMCS_TxQueueTrtcm_S {
	unchar					tsIdx ;
	ushort					cirValue ;
	ushort					cbsUnit ;
	ushort					pirValue ;
	ushort					pbsUnit ;
} ;
***************************************************************/
static int xmcs_get_tx_trtcm_params(struct XMCS_TxQueueTrtcm_S *pTrtcmParams)
{
	QDMA_TxQueueTrtcm_T qdmaTrtcmParams ;
	int ret = 0 ;
	
	if(pTrtcmParams->tsIdx<0 || pTrtcmParams->tsIdx>=XPON_SHAPING_NUMBER) {
		return -EINVAL ;
	}

	qdmaTrtcmParams.tsid = pTrtcmParams->tsIdx ;
#ifndef TCSUPPORT_CPU_EN7521
	ret = qdma_get_txqueue_trtcm_params(&qdmaTrtcmParams) ;
#endif
	if(ret != 0) {
		return ret ;
	}
		
	pTrtcmParams->cirValue = qdmaTrtcmParams.cirParamValue ;
	pTrtcmParams->cbsUnit = qdmaTrtcmParams.cbsParamValue ;
	pTrtcmParams->pirValue = qdmaTrtcmParams.pirParamValue ;
	pTrtcmParams->pbsUnit = qdmaTrtcmParams.pbsParamValue ;
	
	return 0 ;
}

/***************************************************************
typedef enum {
	XMCS_IF_PCP_TYPE_CDM_TX = 0,
	XMCS_IF_PCP_TYPE_CDM_RX, 
	XMCS_IF_PCP_TYPE_GDM_TX
} XMCSIF_PcpType_t ;
typedef enum {
	XMCS_IF_PCP_MODE_DISABLE = 0,
	XMCS_IF_PCP_MODE_8B0D,
	XMCS_IF_PCP_MODE_7B1D, 
	XMCS_IF_PCP_MODE_6B2D,
	XMCS_IF_PCP_MODE_5B3D
} XMCSIF_PcpMode_t ;
struct XMCS_TxPCPConfig_S {
	XMCSIF_PcpType_t		pcpType ;
	XMCSIF_PcpMode_t		pcpMode ;
} ;
***************************************************************/
static int xmcs_set_pcp_config(struct XMCS_TxPCPConfig_S *pPcpCfg)
{
	PSE_PcpType_t psePcpType ;
	PSE_PcpMode_t psePcpMode ;
	
	if(pPcpCfg->pcpType == XMCS_IF_PCP_TYPE_CDM_TX) {
		psePcpType = PSE_PCP_TYPE_CDM_TX ;
	} else if(pPcpCfg->pcpType == XMCS_IF_PCP_TYPE_CDM_RX) {
		psePcpType = PSE_PCP_TYPE_CDM_RX ;
	} else if(pPcpCfg->pcpType == XMCS_IF_PCP_TYPE_GDM_RX) {
		psePcpType = PSE_PCP_TYPE_GDM_RX ;
	} else {
		return -EINVAL ;
	}
	
	if(pPcpCfg->pcpMode == XMCS_IF_PCP_MODE_DISABLE) {
		psePcpMode = PSE_PCP_MODE_DISABLE ;
	} else if(pPcpCfg->pcpMode == XMCS_IF_PCP_MODE_8B0D) {
		psePcpMode = PSE_PCP_MODE_8B0D ;
	} else if(pPcpCfg->pcpMode == XMCS_IF_PCP_MODE_7B1D) {
		psePcpMode = PSE_PCP_MODE_7B1D ;
	} else if(pPcpCfg->pcpMode == XMCS_IF_PCP_MODE_6B2D) {
		psePcpMode = PSE_PCP_MODE_6B2D ;
	} else if(pPcpCfg->pcpMode == XMCS_IF_PCP_MODE_5B3D) {
		psePcpMode = PSE_PCP_MODE_5B3D ;
	} else {
		return -EINVAL ;
	} 
	
#ifdef TCSUPPORT_CPU_EN7521
	//todo: not implement yet
#else
	return qdma_pse_set_pcp_config(psePcpType, psePcpMode) ;
#endif
}

/***************************************************************
typedef enum {
	XMCS_IF_PCP_TYPE_CDM_TX = 0,
	XMCS_IF_PCP_TYPE_CDM_RX, 
	XMCS_IF_PCP_TYPE_GDM_TX
} XMCSIF_PcpType_t ;
typedef enum {
	XMCS_IF_PCP_MODE_DISABLE = 0,
	XMCS_IF_PCP_MODE_8B0D,
	XMCS_IF_PCP_MODE_7B1D, 
	XMCS_IF_PCP_MODE_6B2D,
	XMCS_IF_PCP_MODE_5B3D
} XMCSIF_PcpMode_t ;
struct XMCS_TxPCPConfig_S {
	XMCSIF_PcpType_t		pcpType ;
	XMCSIF_PcpMode_t		pcpMode ;
} ;
***************************************************************/
static int xmcs_get_pcp_config(struct XMCS_TxPCPConfig_S *pPcpCfg)
{
	PSE_PcpType_t psePcpType ;
	PSE_PcpMode_t psePcpMode ;
	
	if(pPcpCfg->pcpType == XMCS_IF_PCP_TYPE_CDM_TX) {
		psePcpType = PSE_PCP_TYPE_CDM_TX ;
	} else if(pPcpCfg->pcpType == XMCS_IF_PCP_TYPE_CDM_RX) {
		psePcpType = PSE_PCP_TYPE_CDM_RX ;
	} else if(pPcpCfg->pcpType == XMCS_IF_PCP_TYPE_GDM_RX) {
		psePcpType = PSE_PCP_TYPE_GDM_RX ;
	} else {
		return -EINVAL ;
	}
#ifdef TCSUPPORT_CPU_EN7521
	//todo: not implement yet
#else
	psePcpMode = qdma_pse_get_pcp_config(psePcpType) ;
#endif
	if(psePcpMode == PSE_PCP_MODE_DISABLE) {
		pPcpCfg->pcpMode = XMCS_IF_PCP_MODE_DISABLE ;
	} else if(psePcpMode == PSE_PCP_MODE_8B0D) {
		pPcpCfg->pcpMode = XMCS_IF_PCP_MODE_8B0D ;
	} else if(psePcpMode == PSE_PCP_MODE_7B1D) {
		pPcpCfg->pcpMode = XMCS_IF_PCP_MODE_7B1D ;
	} else if(psePcpMode == PSE_PCP_MODE_6B2D) {
		pPcpCfg->pcpMode = XMCS_IF_PCP_MODE_6B2D ;
	} else if(psePcpMode == PSE_PCP_MODE_5B3D) {
		pPcpCfg->pcpMode = XMCS_IF_PCP_MODE_5B3D ;
	} else {
		return -EFAULT ;
	} 
	
	return 0 ;
}

void prepare_epon(void){
    /* Set the SCU register to select the WAN mode (EPON) */
    XPON_PHY_FW_READY_DISABLE();
	select_xpon_wan(PHY_EPON_CONFIG);
    XPON_PHY_SET_MODE(PHY_EPON_CONFIG);
    XPON_PHY_FW_READY_ENABLE();
    //clear p6 egress traffic limit
    regWrite32(0xbfb59640,0x0);
}

int xmcs_set_connection_start(XPON_Mode_t mode)
{


    XPON_DPRINT_MSG("mode=%d\n", mode);
    /* notify XPON PHY Driver get ready to handle interrupts*/

    if(XPON_ENABLE == mode){
        XPON_PHY_SET(PON_SET_PHY_START) ;
        gpPonSysData->sysStartup = PON_WAN_START ;
        if(PHY_GPON_CONFIG==gpPhyData->working_mode){
            if((1==gpMcsPriv->ctrlFlag.report_init_O1)||        \
               (PHY_LOS_HAPPEN == XPON_PHY_GET(PON_GET_PHY_LOS_STATUS))){      
                xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_STATE_CHANGE, GPON_STATE_O1);
                }
        }
        XPON_START_TIMER(gpPhyData->trans_status_refresh_timer) ;

		if(PHY_UNKNOWN_CONFIG == gpPhyData->working_mode){
            schedule_phy_mode_auto_detection();
        }else{
            XPON_DPRINT_MSG("gpPhyData->working_mode: %d\n", gpPhyData->working_mode);
            switch(gpPhyData->working_mode)
            {
                case PHY_GPON_CONFIG:
                    prepare_gpon();
                    break;

                case PHY_EPON_CONFIG:
                    prepare_epon();
                    break;

                default:
                    dump_stack();
                    printk("Can't recognize phy working mode: %d\n",gpPhyData->working_mode);
                    break;
            }

        	if (XPON_ROUGE_STATE_TRUE == gpPonSysData->rogue_state) {
        		XPON_PHY_SET_ROGUE(PHY_TX_ROGUE_MODE) ;
        	}
        }        
	}else if((mode == XPON_DISABLE) || (mode ==XPON_POWER_DOWN) ) {
            stop_phy_mode_detect();
            XPON_PHY_SET(PON_SET_PHY_STOP) ;
            gpPonSysData->sysStartup = PON_WAN_STOP ;
            gpPhyData->phy_link_status = PHY_LINK_STATUS_LOS;

            switch(gpPhyData->working_mode)
            {
                case PHY_GPON_CONFIG:
        			gpon_disable() ;
                    break;

                case PHY_EPON_CONFIG:
			        eponStop() ;
                    break;

                default:
                    break;
            }
            xpon_stop_timer() ;
    }

	/*enable or disable transceiver power*/
	if(mode == XPON_POWER_DOWN || gpGponPriv->emergencystate){
        XPON_PHY_TX_DISABLE();
    }
    else{
        XPON_PHY_TX_ENABLE();
    }

	return 0 ;
}

/******************************************************************************
******************************************************************************/
int xmcs_get_onu_type(XMCSIF_OnuType_t *type)
{
	if(NULL == type)
		return -EINVAL ;

#if !defined(TCSUPPORT_XPON_HAL_API_EXT) 
	if(gpPonSysData->sysOnuType == PON_ONU_TYPE_HGU) {
		*type = XMCS_IF_ONU_TYPE_HGU;
	} else if(gpPonSysData->sysOnuType == PON_ONU_TYPE_SFU) {
		*type = XMCS_IF_ONU_TYPE_SFU;
	} else {
		*type = XMCS_IF_ONU_TYPE_UNKNOWN;
	} 
#endif
	return 0;
}
EXPORT_SYMBOL(xmcs_get_onu_type) ;

/***************************************************************
***************************************************************/
int xmcs_set_link_detection(XMCSIF_WanDetectionMode_t detection)
{
	if(gpPonSysData->sysStartup == PON_WAN_START) {
		return 0;
	}
	
    XPON_DPRINT_MSG("detection: %d\n", detection);
    
    switch(detection)
    {
        case XMCS_IF_WAN_DETECT_MODE_GPON:
            gpPhyData->is_fix_mode  = TRUE;
            gpPhyData->working_mode = PHY_GPON_CONFIG;
            break;

        case XMCS_IF_WAN_DETECT_MODE_EPON:
            gpPhyData->is_fix_mode  = TRUE;
            gpPhyData->working_mode = PHY_EPON_CONFIG;
            break;

        case XMCS_IF_WAN_DETECT_MODE_AUTO:
            gpPhyData->is_fix_mode  = FALSE;
            break;

        default:
            return -EINVAL;
	}

    return 0;
}


#ifdef TCSUPPORT_AUTOBENCH
EXPORT_SYMBOL(xmcs_set_link_detection) ;
EXPORT_SYMBOL(xmcs_set_connection_start) ;
#endif

/******************************************************************************
******************************************************************************/

int xmcs_get_wan_link_status(struct XMCS_WanLinkConfig_S *pSysLinkStatus)
{
	pSysLinkStatus->linkStart = (gpPonSysData->sysStartup==PON_WAN_START) ? XPON_ENABLE : XPON_DISABLE ;
	
	if(TRUE == gpPhyData->is_fix_mode) {
        pSysLinkStatus->detectMode =  
            ((PHY_GPON_CONFIG == gpPhyData->working_mode) ? 
            XMCS_IF_WAN_DETECT_MODE_GPON :
            XMCS_IF_WAN_DETECT_MODE_EPON );
        
	} else {
		pSysLinkStatus->detectMode = XMCS_IF_WAN_DETECT_MODE_AUTO ;
	} 

	if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) {
		pSysLinkStatus->linkStatus = XMCS_IF_WAN_LINK_GPON ;
	} else if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON) {
		pSysLinkStatus->linkStatus = XMCS_IF_WAN_LINK_EPON ;
	} else {
		pSysLinkStatus->linkStatus = XMCS_IF_WAN_LINK_OFF ;
	} 

	return XMCS_IF_WAN_LINK_OFF ;
}
EXPORT_SYMBOL(xmcs_get_wan_link_status) ;

/******************************************************************************
** 				mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01   
******************************************************************************/
int xmcs_set_wan_cnt_stats(uint mask)
{
#ifdef TCSUPPORT_CPU_EN7521
	if(mask&0x01)
		FE_API_SET_CLEAR_MIB(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX);
	else if(mask&0x02)
		FE_API_SET_CLEAR_MIB(FE_GDM_SEL_GDMA2, FE_GDM_SEL_RX);
#else
	uint value = fe_reg_read(GDMA2_MIB_CLR - FE_BASE);
	value |= (mask & 0x03);
	fe_reg_write((GDMA2_MIB_CLR - FE_BASE) , value);
#endif
	return 0;
}

/******************************************************************************
******************************************************************************/
int xmcs_get_wan_cnt_stats(struct XMCS_WanCntStats_S *pWanCntStats)
{
#ifdef TCSUPPORT_WAN_GPON
	REG_DBG_GEM_HEC_ONE_ERR_CNT gponHecOneCnt;
	REG_DBG_GEM_HEC_TWO_ERR_CNT gponHecTwoCnt;
	REG_DBG_GEM_HEC_UC_ERR_CNT gponHecUcCnt;
#endif
	PHY_FecCount_T phyFecCount ;
	if (pWanCntStats == NULL)
		return -EINVAL;

    struct XMCS_WanCntStats_S wan_cnt_temp = {0};
    
#ifdef TCSUPPORT_WAN_GPON
	if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) {
		gponHecOneCnt.Raw = IO_GREG(DBG_GEM_HEC_ONE_ERR_CNT) ;
		gponHecTwoCnt.Raw = IO_GREG(DBG_GEM_HEC_TWO_ERR_CNT) ;
		gponHecUcCnt.Raw = IO_GREG(DBG_GEM_HEC_UC_ERR_CNT) ;

		wan_cnt_temp.rxHecErrorCnt = gponHecOneCnt.Bits.dbg_gem_one_err_cnt + \
			gponHecTwoCnt.Bits.dbg_gem_two_err_cnt + gponHecUcCnt.Bits.dbg_gem_uc_err_cnt ;
	}else{
		wan_cnt_temp.rxHecErrorCnt = 0;
	}
#endif

    
    XPON_PHY_GET_API(PON_GET_PHY_RX_FEC_COUNTER, &phyFecCount);
	wan_cnt_temp.rxFecErrorCnt = phyFecCount.uncorrect_codewords;
	wan_cnt_temp.rxFecCerrorCnt= phyFecCount.correct_codewords;
    wan_cnt_temp.FecSeconds    = phyFecCount.fec_seconds;
	wan_cnt_temp.BipError      = XPON_PHY_GET(PON_GET_PHY_BIP_COUNTER);
#ifdef TCSUPPORT_CPU_EN7521
	FE_TxCnt_t txCnt;
	FE_RxCnt_t rxCnt;
	FE_API_GET_ETH_TX_CNT(&txCnt);
	FE_API_GET_ETH_RX_CNT(&rxCnt);
	wan_cnt_temp.txFrameCnt 		= txCnt.txFrameCnt;
	wan_cnt_temp.txFrameLen 		= txCnt.txFrameLen;
	wan_cnt_temp.txDropCnt 			= txCnt.txDropCnt;
	wan_cnt_temp.txBroadcastCnt 	= txCnt.txBroadcastCnt;
	wan_cnt_temp.txMulticastCnt 	= txCnt.txMulticastCnt;
	wan_cnt_temp.txLess64Cnt 		= txCnt.txLess64Cnt;
	wan_cnt_temp.txMore1518Cnt 		= txCnt.txMore1518Cnt;
	wan_cnt_temp.tx64Cnt 			= txCnt.txEq64Cnt;
	wan_cnt_temp.tx65To127Cnt 		= txCnt.txFrom65To127Cnt;
	wan_cnt_temp.tx128To255Cnt 		= txCnt.txFrom128To255Cnt;
	wan_cnt_temp.tx256To511Cnt 		= txCnt.txFrom256To511Cnt;
	wan_cnt_temp.tx512To1023Cnt 	= txCnt.txFrom512To1023Cnt;
	wan_cnt_temp.tx1024To1518Cnt 	= txCnt.txFrom1024To1518Cnt;

	wan_cnt_temp.rxFrameCnt 		= rxCnt.rxFrameCnt;
	wan_cnt_temp.rxFrameLen 		= rxCnt.rxFrameLen;
	wan_cnt_temp.rxDropCnt 			= rxCnt.rxDropCnt;
	wan_cnt_temp.rxBroadcastCnt		= rxCnt.rxBroadcastCnt;
	wan_cnt_temp.rxMulticastCnt 	= rxCnt.rxMulticastCnt;
	wan_cnt_temp.rxCrcCnt 			= rxCnt.rxCrcCnt;
	wan_cnt_temp.rxFragFameCnt 		= rxCnt.rxFragFameCnt;
	wan_cnt_temp.rxJabberFameCnt	= rxCnt.rxJabberFameCnt;
	wan_cnt_temp.rxLess64Cnt 		= rxCnt.rxLess64Cnt;
	wan_cnt_temp.rxMore1518Cnt 		= rxCnt.rxMore1518Cnt;
	wan_cnt_temp.rx64Cnt 			= rxCnt.rxEq64Cnt;
	wan_cnt_temp.rx65To127Cnt 		= rxCnt.rxFrom65To127Cnt;
	wan_cnt_temp.rx128To255Cnt 		= rxCnt.rxFrom128To255Cnt;
	wan_cnt_temp.rx256To511Cnt 		= rxCnt.rxFrom256To511Cnt;
	wan_cnt_temp.rx512To1023Cnt 	= rxCnt.rxFrom512To1023Cnt;
	wan_cnt_temp.rx1024To1518Cnt 	= rxCnt.rxFrom1024To1518Cnt;
#else
	wan_cnt_temp.txFrameCnt 		= fe_reg_read(GDMA2_TX_ETHCNT - FE_BASE);
	wan_cnt_temp.txFrameLen 		= fe_reg_read(GDMA2_TX_ETHLENCNT - FE_BASE);
	wan_cnt_temp.txDropCnt 			= fe_reg_read(GDMA2_TX_ETHDROPCNT - FE_BASE);
	wan_cnt_temp.txBroadcastCnt 	= fe_reg_read(GDMA2_TX_ETHBCDCNT - FE_BASE);
	wan_cnt_temp.txMulticastCnt 		= fe_reg_read(GDMA2_TX_ETHMULTICASTCNT - FE_BASE);
	wan_cnt_temp.txLess64Cnt 		= fe_reg_read(GDMA2_TX_ETH_LESS64_CNT - FE_BASE);
	wan_cnt_temp.txMore1518Cnt 	= fe_reg_read(GDMA2_TX_ETH_MORE1518_CNT - FE_BASE);
	wan_cnt_temp.tx64Cnt 			= fe_reg_read(GDMA2_TX_ETH_64_CNT - FE_BASE);
	wan_cnt_temp.tx65To127Cnt 		= fe_reg_read(GDMA2_TX_ETH_65_TO_127_CNT - FE_BASE);
	wan_cnt_temp.tx128To255Cnt 	= fe_reg_read(GDMA2_TX_ETH_128_TO_255_CNT - FE_BASE);
	wan_cnt_temp.tx256To511Cnt 	= fe_reg_read(GDMA2_TX_ETH_256_TO_511_CNT - FE_BASE);
	wan_cnt_temp.tx512To1023Cnt 	= fe_reg_read(GDMA2_TX_ETH_512_TO_1023_CNT - FE_BASE);
	wan_cnt_temp.tx1024To1518Cnt 	= fe_reg_read(GDMA2_TX_ETH_1024_TO_1518_CNT - FE_BASE);

	wan_cnt_temp.rxFrameCnt 		= fe_reg_read(GDMA2_RX_ETHERPCNT - FE_BASE);
	wan_cnt_temp.rxFrameLen 		= fe_reg_read(GDMA2_RX_ETHERPLEN - FE_BASE);
	wan_cnt_temp.rxDropCnt 			= fe_reg_read(GDMA2_RX_ETHDROPCNT - FE_BASE);
	wan_cnt_temp.rxBroadcastCnt		= fe_reg_read(GDMA2_RX_ETHBCCNT - FE_BASE);
	wan_cnt_temp.rxMulticastCnt 		= fe_reg_read(GDMA2_RX_ETHMCCNT - FE_BASE);
	wan_cnt_temp.rxCrcCnt 			= fe_reg_read( GDMA2_RX_ETHCRCCNT - FE_BASE);
	wan_cnt_temp.rxFragFameCnt 	= fe_reg_read(GDMA2_RX_ETHFRACCNT - FE_BASE);
	wan_cnt_temp.rxJabberFameCnt	= fe_reg_read(GDMA2_RX_ETHJABCNT - FE_BASE);
	wan_cnt_temp.rxLess64Cnt 		= fe_reg_read(GDMA2_RX_ETHRUNTCNT - FE_BASE);
	wan_cnt_temp.rxMore1518Cnt 	= fe_reg_read(GDMA2_RX_ETHLONGCNT - FE_BASE);
	wan_cnt_temp.rx64Cnt 			= fe_reg_read(GDMA2_RX_ETH_64_CNT - FE_BASE);
	wan_cnt_temp.rx65To127Cnt 		= fe_reg_read(GDMA2_RX_ETH_65_TO_127_CNT - FE_BASE);
	wan_cnt_temp.rx128To255Cnt 	= fe_reg_read(GDMA2_RX_ETH_128_TO_255_CNT - FE_BASE);
	wan_cnt_temp.rx256To511Cnt 	= fe_reg_read(GDMA2_RX_ETH_256_TO_511_CNT - FE_BASE);
	wan_cnt_temp.rx512To1023Cnt 	= fe_reg_read(GDMA2_RX_ETH_512_TO_1023_CNT - FE_BASE);
	wan_cnt_temp.rx1024To1518Cnt 	= fe_reg_read(GDMA2_RX_ETH_1024_TO_1518_CNT - FE_BASE); 
#endif
    copy_to_user(pWanCntStats, &wan_cnt_temp, sizeof(wan_cnt_temp) );

	return 0;
}

/******************************************************************************
******************************************************************************/
int xmcs_get_ploam_gtc_info(PLOAM_GTC_INFO_T * gtc_info)
{
    PLOAM_GTC_INFO_T temp = {0};
    REG_G_PLOu_GUARD_BIT gponGuardBit = {0};
	REG_G_PLOu_PRMBL_TYPE1_2 gponPrmblType1_2 = {0};
	REG_G_PLOu_PRMBL_TYPE3 gponPrmblType3 = {0};
	uint regDelimiter = 0; 
	int ret = -1;
    if(gtc_info == NULL)
		return -1;
	gponGuardBit.Raw = IO_GREG(G_PLOu_GUARD_BIT);
    temp.numGuardBits = gponGuardBit.Bits.guard_bit;
	gponPrmblType1_2.Raw = IO_GREG(G_PLOu_PRMBL_TYPE1_2);
	temp.numType1PreambleBits = gponPrmblType1_2.Bits.prmb1_bit;
	temp.numType2PreambleBits = gponPrmblType1_2.Bits.prmb2_bit;
	gponPrmblType3.Raw = IO_GREG(G_PLOu_PRMBL_TYPE3);
	if(GPON_CURR_STATE == GPON_STATE_O3 || GPON_CURR_STATE == GPON_STATE_O4)
	{
	    temp.numType3PreambleBits = gponPrmblType3.Bits.ext_prmb3_o3_o4_num;
	}
	else if(GPON_CURR_STATE == GPON_STATE_O5)
	{
	    temp.numType3PreambleBits = gponPrmblType3.Bits.ext_prmb3_o5_num;
	}
	else
	{
	    temp.numType3PreambleBits = 0;
	}	
	regDelimiter = IO_GREG(G_PLOu_DELM_BIT);
	temp.delimiterData[0] = regDelimiter >> 16;
	temp.delimiterData[1] = (regDelimiter >> 8) & 255;
	temp.delimiterData[2] = regDelimiter & 255;
	if(gpGponPriv == NULL)
	    return -1;
	temp.type3PreamblePattern = gpGponPriv->gponCfg.preamble_t3_pat;
    if(gpGponPriv->gponCfg.tx_power == 0)
    {
        temp.powerLevel = SC_PLOAM_POWER_LEVEL_NORMAL;
    }
	else if(gpGponPriv->gponCfg.tx_power == 1)
	{
	    temp.powerLevel = SC_PLOAM_POWER_LEVEL_MIN_3DB;
	}
	else if(gpGponPriv->gponCfg.tx_power == 2)
	{
	    temp.powerLevel = SC_PLOAM_POWER_LEVEL_MIN_6DB;
	}
	else
	{
	    temp.powerLevel = SC_PLOAM_POWER_LEVEL_OTHER;
	}
	temp.eqd = IO_GREG(G_EQD);
	ret = copy_to_user(gtc_info, &temp, sizeof(PLOAM_GTC_INFO_T));
	if(0 != ret)
	{
	    printk("copy_to_user failed\n");
		return -1;
	}
    return 0;
}

int xmcs_get_drv_power_level(PLOAM_POWER_LEVEL_T * level)
{
    PLOAM_POWER_LEVEL_T temp = SC_PLOAM_POWER_LEVEL_OTHER;
	if(level == NULL)
		return -1;
    if(gpGponPriv == NULL)
	    return -1;
    if(gpGponPriv->gponCfg.tx_power == 0)
    {
        temp = SC_PLOAM_POWER_LEVEL_NORMAL;
    }
	else if(gpGponPriv->gponCfg.tx_power == 1)
	{
	    temp = SC_PLOAM_POWER_LEVEL_MIN_3DB;
	}
	else if(gpGponPriv->gponCfg.tx_power == 2)
	{
	    temp = SC_PLOAM_POWER_LEVEL_MIN_6DB;
	}
	else
	{
	    temp = SC_PLOAM_POWER_LEVEL_OTHER;
	}
    if(0 != put_user(temp, level))
	{
	    printk("put_user failed\n");
		return -1;
	}
    return 0;
}

#ifdef TCSUPPORT_CPU_EN7521
int xmcs_get_all_tcont_packet_info(ALL_TCONT_COUNTERS_T * pallcounters)
{
    int i;
	int j = 0;
	ALL_TCONT_COUNTERS_T temp = {0};
	int ret = -1;

    if(pallcounters == NULL)
		return -1;
    for(i = 0; i < CONFIG_GPON_MAX_TCONT; i++)
    {
        if(gpWanPriv->gpon.allocId[i] == GPON_UNASSIGN_ALLOC_ID)
		{
		    continue;
	    }
		ret = QDMA_API_DUMP_CNTR_CHANNEL(ECNT_QDMA_WAN, i);
		if(ret == -1)
		{
		    printk("QDMA_API_DUMP_CNTR_CHANNEL channel_id %d return value error\n", i);
			continue;
		}
		temp.counters[j].alloc_id = gpWanPriv->gpon.allocId[i];
		temp.counters[j].tx_counter = ret;
		j++;
    }
	temp.tcont_num = j;
	if(temp.tcont_num == 0)
	{
	    printk("no valid tcont\n");
		return 0;
	}
	if(0 != copy_to_user(pallcounters, &temp, sizeof(ALL_TCONT_COUNTERS_T)))
	{
	    printk("copy_to_user failed\n");
		return -1;
	}
    return 0;
}
	
int xmcs_get_tcont_packet_info(TCONT_COUNTERS_T * pcounters)
{
    int i;
	int ret = -1;
	TCONT_COUNTERS_T temp = {0};
	
    if(pcounters == NULL)
		return -1;
    ret= copy_from_user(&temp, pcounters, sizeof(TCONT_COUNTERS_T));
	if(0 != ret)
	{
	    printk("copy_from_user failed\n");
		return -1;
	} 
    for(i = 0; i < CONFIG_GPON_MAX_TCONT; i++)
	{
        if(temp.alloc_id == gpWanPriv->gpon.allocId[i])
        {
			break;
        }
    }

	if(CONFIG_GPON_MAX_TCONT == i)
	{
	    printk("invalid allocid %d\n", temp.alloc_id);
	    return -1;
	}
	ret = QDMA_API_DUMP_CNTR_CHANNEL(ECNT_QDMA_WAN, i);
	if (ret == -1)
	{
	    printk("QDMA_API_DUMP_CNTR_CHANNEL allocid %hu return value error\n", temp.alloc_id);
		return -1;
	}
	temp.tx_counter = ret;
	if(0 != copy_to_user(pcounters, &temp, sizeof(TCONT_COUNTERS_T)))
	{
	    printk("copy_to_user failed\n");
		return -1;
	}
	return 0;
}
#endif


/*****************************************************************************
******************************************************************************/
int xpon_reset_qdma_tx_buf(void ){
	QDMA_TxBufCtrl_T txBufCtrl;

	txBufCtrl.mode = QDMA_ENABLE;
	txBufCtrl.chnThreshold = min(0x20, 0x80/gpWanPriv->activeChannelNum);
	txBufCtrl.totalThreshold = 0x80;
	
#ifdef TCSUPPORT_CPU_EN7521
	QDMA_API_SET_TXBUF_THRESHOLD(ECNT_QDMA_WAN, &txBufCtrl);
#else
	qdma_set_txbuf_threshold(&txBufCtrl);
#endif

	return 0;
}
EXPORT_SYMBOL(xpon_reset_qdma_tx_buf) ;
/* this global value is for Fiberhome CIG test, to disable  isTxDropOmcc flag*/
__u8 green_drop_flag = TRUE;

#ifdef TCSUPPORT_CPU_EN7521
int xpon_set_qdma_qos(int enable)
{
	if (enable == 0){ 
		gpWanPriv->devCfg.flags.isQosUp = 0;
	}else{ 
		gpWanPriv->devCfg.flags.isQosUp = 1;
	}
}
#else

 extern unsigned int hw_qos_enable; 

int xpon_set_qdma_qos(int enable)
{
	int i =0;
	QDMA_TxBufCtrl_T txBufCtrl;
	QDMA_Mode_t mode ;
	QDMA_TxQueueCongestThreshold_T setThreshold ;
	QDMA_TxQueueCongestScale_T setScale;

#if !defined(TCSUPPORT_XPON_HAL_API_EXT) 
#if defined(TCSUPPORT_GPON_GREENDROP_OPEN)
	enable = 1;
#else
#endif	
#endif

	if (enable == 0){ 
		gpWanPriv->devCfg.flags.isQosUp = 0;
	}else{ 
		gpWanPriv->devCfg.flags.isQosUp = 1;
	}	

#if !defined(TCSUPPORT_CT_QOS) || !defined(TCSUPPORT_CT_PON_C7) 
	if (gpWanPriv->devCfg.flags.isTxDropOmcc && green_drop_flag)
#endif
        {
		enable = 1;
	}
	/* step 1: limite PSE buffer usage for each channel, then
	 * low priority packes won't block high priority
	 * packets (in PSE buffer) for too long, in order
	 * to prevent highest priority packets from dropping */
	txBufCtrl.mode = QDMA_ENABLE;
	txBufCtrl.chnThreshold = min(0x20, 0x80/gpWanPriv->activeChannelNum);
	txBufCtrl.totalThreshold = 0x80;
			
	qdma_set_txbuf_threshold(&txBufCtrl);

	/* step2: do QDMA congestion configuration, otherwise low
	 * priority packets may use all tx descriptors. 
	 * We suppose each queue (for all channels) can 
	 * use 8*15==120 dscps at most! */
	if (enable == 0){
		mode = QDMA_DISABLE;
	}else{
		setScale.minScale = QDMA_TXQUEUE_SCALE_2_DSCP;
		if (gpWanPriv->activeChannelNum > 8)
			setScale.maxScale = QDMA_TXQUEUE_SCALE_8_DSCP;
		else 
			setScale.maxScale = QDMA_TXQUEUE_SCALE_16_DSCP;
			
		qdma_set_congestion_scale(&setScale);
		
		mode = QDMA_ENABLE;
		for (i =0; i < XPON_QUEUE_NUMBER; i++){
			setThreshold.queueIdx = i;
			setThreshold.grnMaxThreshold = gpWanPriv->greenMaxthreshold;
			setThreshold.grnMinThreshold = 0;
			setThreshold.ylwMaxThreshold = 0;
			setThreshold.ylwMinThreshold = 0;
			qdma_set_congest_threshold(&setThreshold);
		}		
	} 


	//printk("%s QDMA Tx Qos\n", (enable)?"Enable":"Disable");
	return 0;
	
}
#endif
EXPORT_SYMBOL(xpon_set_qdma_qos) ;

int xpon_set_qos(u8 enable, u8 queueNum){
	
	if (enable == DISABLE) {
		gpWanPriv->devCfg.flags.isQosUp = XPON_DISABLE;
		xpon_set_qdma_qos(XPON_DISABLE);
	} else if (enable == ENABLE){
		gpWanPriv->devCfg.flags.isQosUp = XPON_ENABLE;
		if ((queueNum < 9) && (queueNum > 4)) {
			gpWanPriv->greenMaxthreshold = 64/queueNum;
		}	
		xpon_set_qdma_qos(XPON_ENABLE);
	} else {
		printk("xpon_set_qos:wrong value\n");
	}

	return 0;
}

EXPORT_SYMBOL(xpon_set_qos) ;

int mulitcast_ani = -1;
EXPORT_SYMBOL(mulitcast_ani);

#ifdef TCSUPPORT_WAN_GPON
/***************************************************************
***************************************************************/
static int xmcs_assign_gem_port(struct XMCS_GemPortAssign_S *pGemAssign)
{
	int i, ret ;
	unchar gemIdx;
	ushort gemPortId;
	if(pGemAssign==NULL || pGemAssign->entryNum>=CONFIG_GPON_MAX_GEMPORT) {
		return -EINVAL ;
	}
	
	for(i=0 ; i<pGemAssign->entryNum ; i++) {
		if(pGemAssign->gemPort[i].ani >= GPON_MAX_ANI_INTERFACE) {
			return -EINVAL ;
		}

		if(!gpWanPriv->pPonNetDev[PWAN_IF_DATA]) {
			return -EINVAL ;
		}
	
		if(pGemAssign->gemPort[i].id<GPON_MAX_GEM_ID && pGemAssign->gemPort[i].id!=GPON_OMCC_ID) {
			ret = gwan_config_gemport(pGemAssign->gemPort[i].id, ENUM_CFG_NETIDX, pGemAssign->gemPort[i].ani) ;
			if(ret != 0) {
				return ret ;
			}
		}
		if(isDasan){
		gemPortId = pGemAssign->gemPort[i].id;
		if(gemPortId >= 0 && gemPortId < GPON_MAX_GEM_ID){
			gemIdx = (gpWanPriv->gpon.gemIdToIndex[gemPortId] & GPON_GEM_IDX_MASK);
			if(gpWanPriv->gpon.gemPort[gemIdx].info.valid && gpWanPriv->gpon.gemPort[gemIdx].info.portId==gemPortId) {
				if(gpWanPriv->gpon.gemPort[gemIdx].info.channel == GPON_MULTICAST_CHANNEL){
					mulitcast_ani = pGemAssign->gemPort[i].ani;
				//	printk("set mulitcast_ani=%d\n",mulitcast_ani);
				}
				
			}
		}
	}
	}
	
	return 0 ;
}

void assignGemportId(ushort gemPortId)
{
	struct XMCS_GemPortAssign_S tGemAssign;
	tGemAssign.entryNum = 1;
	tGemAssign.gemPort[0].id  = gemPortId;
	tGemAssign.gemPort[0].ani = 1;	
	
	xmcs_assign_gem_port(&tGemAssign);
//	printk("mannual assign gemport %d,\n",gemPortId);
}

/***************************************************************
***************************************************************/
static int xmcs_unassign_gem_port(struct XMCS_GemPortAssign_S *pGemAssign)
{
	int i, ret ;
	unchar gemIdx;
	ushort gemPortId;
	
	if(pGemAssign==NULL || pGemAssign->entryNum>=CONFIG_GPON_MAX_GEMPORT) {
		return -EINVAL ;
	}

	for(i=0 ; i<pGemAssign->entryNum ; i++) {
		if(pGemAssign->gemPort[i].id<GPON_MAX_GEM_ID && pGemAssign->gemPort[i].id!=GPON_OMCC_ID) {
			ret = gwan_config_gemport(pGemAssign->gemPort[i].id, ENUM_CFG_NETIDX, GPON_MAX_ANI_INTERFACE) ;
			if(ret != 0) {
				return ret ;
			}
			if(isDasan){ 
				gemPortId = pGemAssign->gemPort[i].id;
				
				gemIdx = (gpWanPriv->gpon.gemIdToIndex[gemPortId] & GPON_GEM_IDX_MASK);
				if(gpWanPriv->gpon.gemPort[gemIdx].info.valid && gpWanPriv->gpon.gemPort[gemIdx].info.portId==gemPortId) {
					if(gpWanPriv->gpon.gemPort[gemIdx].info.channel == GPON_MULTICAST_CHANNEL){
						mulitcast_ani = -1;
						//printk("set mulitcast_ani=%d\n",mulitcast_ani);
					}
				}
			}
			
		}
	}
	
	return 0 ;
}

/***************************************************************
***************************************************************/
int xmcs_create_gem_port(struct XMCS_GemPortCreate_S *pGemCreate)
{
	unchar gemIdx, channel=CONFIG_GPON_MAX_TCONT ;
	int i ;
	int ret = 0;
	unchar encryption = 0;
	
	if(pGemCreate->gemPortId>=GPON_MAX_GEM_ID || pGemCreate->gemPortId==GPON_OMCC_ID) {
		return -EINVAL ;
	}
	
	if(pGemCreate->gemType == GPON_UNICAST_GEM) {
		
		if(pGemCreate->allocId<0 || pGemCreate->allocId>=GPON_MAX_ALLOC_ID) {
			return -EINVAL ;
		}
		
		for(i=0 ; i<CONFIG_GPON_MAX_TCONT ; i++) {
			if(gpWanPriv->gpon.allocId[i] == pGemCreate->allocId) {
				channel = i ;				
				break ;
			}
		}

		if(i == CONFIG_GPON_MAX_TCONT){
			channel = 0;
		}
		
	} else if(pGemCreate->gemType == GPON_MULTICAST_GEM) {
		channel = GPON_MULTICAST_CHANNEL ;
	} else {
		return -EINVAL ;	
	}

	gemIdx = (gpWanPriv->gpon.gemIdToIndex[pGemCreate->gemPortId] & GPON_GEM_IDX_MASK);
	if(gpWanPriv->gpon.gemPort[gemIdx].info.valid && gpWanPriv->gpon.gemPort[gemIdx].info.portId==pGemCreate->gemPortId) {
		return gwan_config_gemport(pGemCreate->gemPortId, ENUM_CFG_CHANNEL, channel) ;
	} else {
		/* In some case, the OLT could send the Encrypted_PortID PLOAM 
		before creating GEM port via OMCI */
		encryption = (GPON_GEM_ENCRY_MASK == (gpWanPriv->gpon.gemIdToIndex[pGemCreate->gemPortId] & GPON_GEM_ENCRY_MASK))?1:0;
		ret = gwan_create_new_gemport(pGemCreate->gemPortId, channel, encryption);


	}
	return ret;
}
EXPORT_SYMBOL(xmcs_create_gem_port) ;

/***************************************************************
***************************************************************/
int xmcs_remove_gem_port(ushort gemPortId)
{
	if(gemPortId == GPON_OMCC_ID) {
		return -EINVAL ;
	}

	return gwan_remove_gemport(gemPortId) ;
}
EXPORT_SYMBOL(xmcs_remove_gem_port) ;

/***************************************************************
***************************************************************/
int xmcs_remove_all_gem_port(void)
{
	return gwan_remove_all_gemport() ;
}
EXPORT_SYMBOL(xmcs_remove_all_gem_port) ;

/***************************************************************
***************************************************************/
static int xmcs_set_gem_port_loopback(struct XMCS_GemPortLoopback_S *pGemLb)
{
	if(pGemLb->gemPortId>=GPON_MAX_GEM_ID || pGemLb->gemPortId==GPON_OMCC_ID) {
		return -EINVAL ;
	}
	
	if(pGemLb->loopback == XPON_DISABLE) {
		return gwan_config_gemport(pGemLb->gemPortId, ENUM_CFG_LOOPBACK, 0) ;
	} else {
		return gwan_config_gemport(pGemLb->gemPortId, ENUM_CFG_LOOPBACK, 1) ;
	}
	
	return 0 ;
}

/***************************************************************
***************************************************************/
int xmcs_get_gem_port_info(struct XMCS_GemPortInfo_S *pGemInfo)
{
	int i ;
	unchar channel ;
	
	pGemInfo->entryNum = 0 ;
	for(i=0 ; i<CONFIG_GPON_MAX_GEMPORT ; i++) {
		if(gpWanPriv->gpon.gemPort[i].info.valid) {
			channel = gpWanPriv->gpon.gemPort[i].info.channel ;
			
			if(channel == GPON_MULTICAST_CHANNEL) {
				pGemInfo->info[pGemInfo->entryNum].gemType = GPON_MULTICAST_GEM ;
				pGemInfo->info[pGemInfo->entryNum].allocId = 0xFF;
			} else {
				pGemInfo->info[pGemInfo->entryNum].gemType = GPON_UNICAST_GEM ;
				pGemInfo->info[pGemInfo->entryNum].allocId = (channel==GPON_UNKNOWN_CHANNEL) ? 0xFF : gpWanPriv->gpon.allocId[channel] ;
			}
			pGemInfo->info[pGemInfo->entryNum].gemPortId = gpWanPriv->gpon.gemPort[i].info.portId ;
			pGemInfo->info[pGemInfo->entryNum].aniIdx = gpWanPriv->gpon.gemPort[i].info.ani ;
			pGemInfo->info[pGemInfo->entryNum].lbMode = (gpWanPriv->gpon.gemPort[i].info.rxLb) ? XPON_ENABLE : XPON_DISABLE ;
			pGemInfo->info[pGemInfo->entryNum].enMode = (gpWanPriv->gpon.gemPort[i].info.rxEncrypt) ? XPON_ENABLE : XPON_DISABLE ;

			pGemInfo->entryNum++ ;
		}
	}
	
	return 0 ;
}
EXPORT_SYMBOL(xmcs_get_gem_port_info) ;
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
int xmcs_set_qos_policer_creat(struct XMCS_QosPolicerCreat_S *pQosPolicer)
{
	int i, cir;
	QDMA_TxQueueTrtcm_T txqTrtcm;
	QDMA_TxQueueCongestScale_T txqScale;
	QDMA_Mode_t qdmaMode = QDMA_ENABLE;
#ifndef TCSUPPORT_CPU_EN7521
	QDMA_TxQueueCongestThreshold_T txqThreshold;
	QDMA_TxQueueDropProbability_T txqDropRate;
#endif

	printk("qos_policer_creat\n");

    if(!isTotalBWSet)
    {
    	qosFlag = QOS_HW_CAR;
		trtcmEnable = 1;
#ifdef TCSUPPORT_CPU_EN7521
	//todo: not implement yet
#else
	qdma_set_txqueue_trtcm_mode(qdmaMode);
#endif
    }
    /* HWQOS */
    /* pon_trafficShapingSet */
	memset(&txqTrtcm, 0, sizeof(QDMA_TxQueueTrtcm_T));
	txqTrtcm.tsid = CAR_QUEUE_NUM + 1 - pQosPolicer->policer_id;
	//the uint of cir, pir is 64Kpbs
	cir = pQosPolicer->cir>>6;	
	if((pQosPolicer->cir%64) > 32 ){
		cir = cir + 1;
	}

	txqTrtcm.cirParamValue = cir;
	txqTrtcm.pirParamValue = 65535;
	txqTrtcm.cbsParamValue = 160;
	txqTrtcm.pbsParamValue = 160;

#ifdef TCSUPPORT_CPU_EN7521
	//todo: not implement yet
#else
	qdma_set_txqueue_trtcm_params(&txqTrtcm);
#endif

	if(0 == gQueueMask)
	{
#ifdef TCSUPPORT_CPU_EN7521
		//todo: not implement yet
#else
		memset(&txqDropRate, 0, sizeof(QDMA_TxQueueDropProbability_T));
		txqDropRate.green = 0;
		txqDropRate.yellow = 255;

		qdma_set_congest_drop_probability(&txqDropRate);

		memset(&txqScale, 0, sizeof(QDMA_TxQueueCongestScale_T));
		txqScale.maxScale = QDMA_TXQUEUE_SCALE_16_DSCP;
		qdma_set_congestion_scale(&txqScale);

		memset(&txqThreshold, 0, sizeof(QDMA_TxQueueCongestThreshold_T));
		for (i = 0; i < TX_QUEUE_NUM; i++)
		{
			txqThreshold.queueIdx = i;
			txqThreshold.grnMaxThreshold = 5;
			txqThreshold.ylwMaxThreshold = 5;

			qdma_set_congest_threshold(&txqThreshold);
		}
		qdma_set_txqueue_threshold_mode(qdmaMode);
#endif
	}
	
	gQueueMask |= 1<<(pQosPolicer->policer_id-1);
		
	printk("gQueueMask = %d\n",gQueueMask);
  
    /* SWQOS */
    if(sw_qosCreatPolicer)
    {
        return sw_qosCreatPolicer(pQosPolicer->policer_id, pQosPolicer->cir);
    }

    return 0;
}

int xmcs_set_qos_policer_delete(struct XMCS_QosPolicerDelete_S *pQosPolicerDelete)
{
	QDMA_TxQueueTrtcm_T txqTrtcm;

	printk("qos_policer_delete\n");

	memset(&txqTrtcm, 0, sizeof(QDMA_TxQueueTrtcm_T));
	txqTrtcm.tsid = CAR_QUEUE_NUM + 1 - pQosPolicerDelete->policer_id;
	txqTrtcm.cirParamValue = 16000;
	txqTrtcm.pirParamValue = 16000;
	txqTrtcm.cbsParamValue = 511;
	txqTrtcm.pbsParamValue = 511;

#ifdef TCSUPPORT_CPU_EN7521
	//todo: not implement yet
#else
	qdma_set_txqueue_trtcm_params(&txqTrtcm);
#endif

	gQueueMask &= (63 - (1<<pQosPolicerDelete->policer_id-1));

    /* SWQOS */
	if(sw_qosDeletePolicer)
	{
	 return sw_qosDeletePolicer(pQosPolicerDelete->policer_id);
	}

	return 0;
}

int xmcs_set_qos_overall_ratelimit(struct XMCS_OverallRatelimitConfig_S *pOverallRatelimit)
{
    if(0 == pOverallRatelimit->bandwidth)
    {
        isTotalBWSet = 0;
        qosFlag = QOS_HW_CAR;
		trtcmEnable = 1;
#ifdef TCSUPPORT_CPU_EN7521
		//todo: not implement yet
#else
		qdma_set_txqueue_trtcm_mode(QDMA_ENABLE);
#endif
    }
	else
	{
	    isTotalBWSet = 1;
		trtcmEnable = 0;
		qosFlag = NULLQOS;
	}
	
	if(sw_qosOverallRatelimit)
	{
	    sw_qosOverallRatelimit(pOverallRatelimit->bandwidth);
	}

    return 0;
}
#endif

/***************************************************************
***************************************************************/
int xmcs_get_tcont_info(struct XMCS_TcontInfo_S *pTcontInfo)
{
	int i ;
	
	pTcontInfo->entryNum = 0 ;
	for(i=0 ; i<CONFIG_GPON_MAX_TCONT ; i++) {
		if(gpWanPriv->gpon.allocId[i] != GPON_UNASSIGN_ALLOC_ID) {
			pTcontInfo->info[pTcontInfo->entryNum].allocId = gpWanPriv->gpon.allocId[i] ;
			pTcontInfo->info[pTcontInfo->entryNum].channel = i ;

			pTcontInfo->entryNum++ ;
		}
	}
	
	return 0 ;
}
EXPORT_SYMBOL(xmcs_get_tcont_info) ;

/***************************************************************
***************************************************************/
int xmcs_remove_tcont_info(ushort allocId)
{	
    if(allocId != gpGponPriv->gponCfg.onu_id) {
		/* Setting the MAC register, set the Alloc-ID unvalid */
		if ( 0 == gwan_remove_tcont(allocId)){
			gpWanPriv->activeChannelNum --;
			if (gpWanPriv->activeChannelNum < 1)
				gpWanPriv->activeChannelNum = 1;
			xpon_reset_qdma_tx_buf();

            return 0;
		}
		
	}
    return 0;
}


#endif /* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
/***************************************************************
***************************************************************/
int xmcs_create_llid(struct XMCS_EponLlidCreate_S *pLlidCreate)
{
	unchar idx = pLlidCreate->idx ;
	
	if(idx >= CONFIG_EPON_MAX_LLID) {
		return -EINVAL ;
	}

	if(gpWanPriv->epon.llid[idx].info.valid) {
		return -EEXIST ;
	} else {
		gpWanPriv->epon.llid[idx].info.llid = pLlidCreate->llid ;
		gpWanPriv->epon.llid[idx].info.channel = idx ;
		gpWanPriv->epon.llid[idx].info.valid = 1 ;
		
		return 0 ;
	}
}
EXPORT_SYMBOL(xmcs_create_llid) ;

/***************************************************************
***************************************************************/
int xmcs_remove_llid(unchar idx)
{
	if(idx >= CONFIG_EPON_MAX_LLID) {
		return -EINVAL ;
	}

	if(!gpWanPriv->epon.llid[idx].info.valid) {
		memset(&gpWanPriv->epon.llid[idx].info, 0, sizeof(EWAN_LlidInfo_T)) ;
		memset(&gpWanPriv->epon.llid[idx].stats, 0, sizeof(struct net_device_stats)) ;
		gpWanPriv->epon.llid[idx].info.rxDrop = 1 ;
		gpWanPriv->epon.llid[idx].info.txDrop = 1 ;
		//pwan_delete_net_interface(idx) ;  
#ifdef TCSUPPORT_EPON_FPGA_WORKAROUND
				gpWanPriv->epon.llid[idx].info.rxDrop = 0 ; //	  1
				gpWanPriv->epon.llid[idx].info.txDrop = 0 ;//	1
				gpWanPriv->epon.llid[idx].info.rxLb = 0 ;//lpbk test
#endif		
		
		return 0 ;
	} else {
		return -EINVAL ;
	}
}
EXPORT_SYMBOL(xmcs_remove_llid) ;

/***************************************************************
***************************************************************/
static int xmcs_set_epon_rx_config(struct XMCS_EponRxConfig_S *pRxCfg)
{
	unchar idx = pRxCfg->idx ;
	
	if(idx >= CONFIG_EPON_MAX_LLID) {
		return -EINVAL ;
	}

	if(gpWanPriv->epon.llid[idx].info.valid) {
		if(pRxCfg->rxMode == EPON_RX_DISCARD) {
			gpWanPriv->epon.llid[idx].info.rxDrop = 1 ;
			gpWanPriv->epon.llid[idx].info.rxLb = 0 ;
		} else if(pRxCfg->rxMode == EPON_RX_LOOPBACK) {
			gpWanPriv->epon.llid[idx].info.rxDrop = 0 ;
			gpWanPriv->epon.llid[idx].info.rxLb = 1 ;
		} else {
			gpWanPriv->epon.llid[idx].info.rxDrop = 0 ;
			gpWanPriv->epon.llid[idx].info.rxLb = 0 ;
		}
	} else {
		return -ENOENT ;
	}

	return 0 ;
}
 
/***************************************************************
***************************************************************/
static int xmcs_set_epon_tx_config(struct XMCS_EponTxConfig_S *pTxCfg)
{
	unchar idx = pTxCfg->idx ;
	
	if(idx >= CONFIG_EPON_MAX_LLID) {
		return -EINVAL ;
	}

	if(gpWanPriv->epon.llid[idx].info.valid) {
		if(pTxCfg->txMode == EPON_TX_DISCARD) {
			gpWanPriv->epon.llid[idx].info.txDrop = 1 ;
		} else {
			gpWanPriv->epon.llid[idx].info.txDrop = 0 ;
		}
	} else {
		return -ENOENT ;
	}

	return 0 ;
}
/***************************************************************
***************************************************************/
int xmcs_set_epon_llid_config(uint index)
{
	struct XMCS_EponTxConfig_S txCfg = {0};
	struct XMCS_EponRxConfig_S rxCfg = {0};

	txCfg.idx = index;
	txCfg.txMode = EPON_TX_FORWARDING;
	xmcs_set_epon_tx_config(&txCfg);
	
	rxCfg.idx = index;
	rxCfg.rxMode = EPON_RX_FORWARDING;
	xmcs_set_epon_rx_config(&rxCfg);
	printk("xmcs set llid %d txmod %d rxmod %d\n",index,txCfg.txMode,rxCfg.rxMode);

	return 0 ;
}

/***************************************************************
***************************************************************/
int xmcs_get_llid_info(struct XMCS_EponLlidInfo_S *pLlidInfo)
{
	int i ;
	
	pLlidInfo->entryNum = 0 ;
	for(i=0 ; i<CONFIG_EPON_MAX_LLID ; i++) {
		if(gpWanPriv->epon.llid[pLlidInfo->entryNum].info.valid) {
			pLlidInfo->info[pLlidInfo->entryNum].idx = i ;
			pLlidInfo->info[pLlidInfo->entryNum].llid = gpWanPriv->epon.llid[i].info.llid ;
			pLlidInfo->info[pLlidInfo->entryNum].channel = gpWanPriv->epon.llid[i].info.channel ;
	
			if(gpWanPriv->epon.llid[i].info.rxDrop) {
				pLlidInfo->info[pLlidInfo->entryNum].rxMode = EPON_RX_DISCARD ;
			} else if(gpWanPriv->epon.llid[i].info.rxLb) {
				pLlidInfo->info[pLlidInfo->entryNum].rxMode = EPON_RX_LOOPBACK ;
			} else {
				pLlidInfo->info[pLlidInfo->entryNum].rxMode = EPON_RX_FORWARDING ;
			}
	
			if(gpWanPriv->epon.llid[i].info.txDrop) {
				pLlidInfo->info[pLlidInfo->entryNum].txMode = EPON_TX_DISCARD ;
			} else {
				pLlidInfo->info[pLlidInfo->entryNum].txMode = EPON_TX_FORWARDING ;
			}
			pLlidInfo->entryNum++ ;
		}
	}
	
	return 0 ;
}
EXPORT_SYMBOL(xmcs_get_llid_info) ;
#endif /* TCSUPPORT_WAN_EPON */

/***************************************************************
***************************************************************/

extern int storm_ctrl_shrehold;
extern int qdma_fwd_timer;

int xmcs_set_storm_ctrl_config(struct XMCS_StormCtrlConfig_S *pStormCfg)
{
	uint threld = pStormCfg->threld;
	uint timer = pStormCfg->timer;
	
	if(threld > 128) {
		return -EINVAL ;
	}
	printk("xmcs_set_storm_ctrl_config: ----- threld = %d\r\n", threld);
	printk("xmcs_set_storm_ctrl_config: ----- timer = %d\r\n", timer);

	storm_ctrl_shrehold = threld;
	
	qdma_fwd_timer = timer;

	return 0 ;
}

EXPORT_SYMBOL(xmcs_set_storm_ctrl_config) ;

/***************************************************************
***************************************************************/
int xmcs_set_dbg_level(struct XMCS_DebugLevel_S *dbg)
{
	if(dbg->enable == XPON_ENABLE) {
		gpPonSysData->debugLevel |= dbg->mask;
	} else if(dbg->enable == XPON_DISABLE){
		gpPonSysData->debugLevel &= ~dbg->mask;
	} else {
		return -1;
	}
	
	return 0 ;
}

/***************************************************************
***************************************************************/
int xmcs_get_storm_ctrl_config(struct XMCS_StormCtrlConfig_S *pStormCfg)
{
	printk("xmcs_get_storm_ctrl_config: ----- storm_ctrl_shrehold = %d\r\n", storm_ctrl_shrehold);
	printk("xmcs_get_storm_ctrl_config: ----- qdma_fwd_timer = %d\r\n", qdma_fwd_timer);

	pStormCfg->threld = storm_ctrl_shrehold;
	pStormCfg->timer = qdma_fwd_timer;

	return 0 ;
}

EXPORT_SYMBOL(xmcs_get_storm_ctrl_config) ;

/***************************************************************
***************************************************************/
int xmcs_set_rogue_state(unchar rogue_mode)
{
    return XPON_PHY_SET_ROGUE(rogue_mode);
}

EXPORT_SYMBOL(xmcs_set_rogue_state);


/***************************************************************
***************************************************************/
int xmcs_get_dbg_level(ushort *level)
{
	*level = gpPonSysData->debugLevel;

	return 0 ;
}


/***************************************************************
***************************************************************/
int if_cmd_proc(uint cmd, ulong arg) 
{
	int ret = -EINVAL ;
	
	if(_IOC_DIR(cmd) == _IOC_WRITE) {
		switch(cmd) {
			case IF_IOS_QOS_WEIGHT_CONFIG :
				ret = xmcs_set_qos_weight_config((struct XMCS_QoSWeightConfig_S *)arg) ;
				break ;	
			case IF_IOS_QOS_SCHEDULER :
				ret = xmcs_set_channel_scheduler((struct XMCS_ChannelQoS_S *)arg) ;
				break ;	
			case IF_IOS_TRTCM_CONGEST_MODE :
				ret = xmcs_set_trtcm_congest_mode((struct XMCS_TxQueueCongestion_S *)arg) ;
				break ;	
			case IF_IOS_DEI_CONGEST_MODE :
				ret = xmcs_set_dei_congest_mode((struct XMCS_TxQueueCongestion_S *)arg) ;
				break ;	
			case IF_IOS_THRESHOLD_CONGEST_MODE :
				ret = xmcs_set_threshold_congest_mode((struct XMCS_TxQueueCongestion_S *)arg) ;
				break ;	
			case IF_IOS_CONGEST_SCALE :
				ret = xmcs_set_congest_scale((struct XMCS_TxQueueCongestion_S *)arg) ;
				break ;
			case IF_IOS_ROGUE_MODE :
				ret = xmcs_set_rogue_state((unchar)arg);
				break;
#ifndef TCSUPPORT_CPU_EN7521
			case IF_IOS_CONGEST_DROP_PROBABILITY :
				ret = xmcs_set_congest_drop_probability((struct XMCS_TxQueueCongestion_S *)arg) ;
				break ;	
#endif
			case IF_IOS_CONGEST_THRESHOLD :
				ret = xmcs_set_congest_threshold((struct XMCS_TxQueueCongestion_S *)arg) ;
				break ;	
			case IO_IOS_TX_TRTCM_SCALE :
				ret = xmcs_set_tx_trtcm_scale((struct XMCS_TxTrtcmScale_S *)arg) ;
				break ;	
			case IO_IOS_TX_TRTCM_PARAMS :
				ret = xmcs_set_tx_trtcm_params((struct XMCS_TxQueueTrtcm_S *)arg) ;
				break ;	
			case IO_IOS_PCP_CONFIG :
				ret = xmcs_set_pcp_config((struct XMCS_TxPCPConfig_S *)arg) ;
				break ;	
			case IO_IOS_WAN_LINK_START :
				ret = xmcs_set_connection_start((XPON_Mode_t)arg) ;
				break ;	
			case IO_IOS_WAN_DETECTION_MODE :
				ret = xmcs_set_link_detection((XMCSIF_WanDetectionMode_t)arg) ;
				break ;	
			case IO_IOS_WAN_CNT_STATS:
				ret = xmcs_set_wan_cnt_stats(arg);
				break;
			case IO_IOS_STORM_CTL_CONFIG:
				ret = xmcs_set_storm_ctrl_config((struct XMCS_StormCtrlConfig_S *)arg);
				break;	
			case IO_IOS_DBG_LEVEL : 
				ret = xmcs_set_dbg_level((struct XMCS_DebugLevel_S *)arg) ;
				break ;
#ifdef TCSUPPORT_WAN_GPON
			case IF_IOS_GEMPORT_ASSIGN : 
				ret = xmcs_assign_gem_port((struct XMCS_GemPortAssign_S *)arg) ;
				break ;
			case IF_IOS_GEMPORT_UNASSIGN : 
				ret = xmcs_unassign_gem_port((struct XMCS_GemPortAssign_S *)arg) ;
				break ;
			case IF_IOS_GEMPORT_CREATE : 
				ret = xmcs_create_gem_port((struct XMCS_GemPortCreate_S *)arg) ;
				break ;
			case IF_IOS_GEMPORT_REMOVE : 
				ret = xmcs_remove_gem_port(arg) ;
				break ;
			case IF_IOS_GEMPORT_REMOVE_ALL : 
				ret = xmcs_remove_all_gem_port() ;
				break ;
			case IF_IOS_GEMPORT_LOOPBACK :
				ret = xmcs_set_gem_port_loopback((struct XMCS_GemPortLoopback_S *)arg) ;
				break ;
			case IF_IOS_TCONT_INFO_REMOVE : 
				ret = xmcs_remove_tcont_info((ushort)arg) ;
				break ; 			
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
            case IO_IOS_QOS_CREAT_POLICER:
	            ret = xmcs_set_qos_policer_creat((struct XMCS_QosPolicerCreat_S *)arg);
	        break;	
            case IO_IOS_QOS_DELETE_POLICER:
	            ret = xmcs_set_qos_policer_delete((struct XMCS_QosPolicerDelete_S *)arg);
	        break;	
            case IO_IOS_QOS_OVERALL_RATELIMIT_CONFIG:
				ret = xmcs_set_qos_overall_ratelimit((struct XMCS_OverallRatelimitConfig_S *)arg);
			break;
#endif			
#endif /* TCSUPPORT_WAN_GPON */
#ifdef TCSUPPORT_WAN_EPON
			case IF_IOS_LLID_CREATE : 
				ret = xmcs_create_llid((struct XMCS_EponLlidCreate_S *)arg) ;
				break ;
			case IF_IOS_LLID_REMOVE : 
				ret = xmcs_remove_llid((unchar)arg) ;
				break ;
			case IF_IOS_LLID_RX_CFG : 
				ret = xmcs_set_epon_rx_config((struct XMCS_EponRxConfig_S *)arg) ;
				break ;
			case IF_IOS_LLID_TX_CFG : 
				ret = xmcs_set_epon_tx_config((struct XMCS_EponTxConfig_S *)arg) ;
				break ;
#endif /* TCSUPPORT_WAN_EPON */
			default:
				PON_MSG(MSG_ERR, "No such I/O command, cmd: %x\n", cmd) ;
				break ;
		}
	} else if(_IOC_DIR(cmd) == _IOC_READ) {
		switch(cmd) {
			case IF_IOG_QOS_WEIGHT_CONFIG :
				ret = xmcs_get_qos_weight_config((struct XMCS_QoSWeightConfig_S *)arg) ;
				break ;	
			case IF_IOG_QOS_SCHEDULER :
				ret = xmcs_get_channel_scheduler((struct XMCS_ChannelQoS_S *)arg) ;
				break ;	
			case IO_IOG_CONGEST_INFO :
				ret = xmcs_get_congest_info((struct XMCS_TxQueueCongestion_S *)arg) ;
				break ;	
			case IO_IOG_TX_TRTCM_SCALE :
				ret = xmcs_get_tx_trtcm_scale((struct XMCS_TxTrtcmScale_S *)arg) ;
				break ;	
			case IO_IOG_TX_TRTCM_PARAMS :
				ret = xmcs_get_tx_trtcm_params((struct XMCS_TxQueueTrtcm_S *)arg) ;
				break ;	
			case IO_IOG_PCP_CONFIG :
				ret = xmcs_get_pcp_config((struct XMCS_TxPCPConfig_S *)arg) ;
				break ;	
			case IO_IOG_WAN_LINK_CONFIG :
				ret = xmcs_get_wan_link_status((struct XMCS_WanLinkConfig_S *)arg) ;
				break ;
			case IO_IOG_ONU_TYPE :
				ret = xmcs_get_onu_type((XMCSIF_OnuType_t *)arg) ;
				break ;
			case IO_IOG_WAN_CNT_STATS:
				ret = xmcs_get_wan_cnt_stats((struct XMCS_WanCntStats_S *)arg);
				break;
			case IO_IOG_STORM_CTL_CONFIG:
				ret = xmcs_get_storm_ctrl_config((struct XMCS_StormCtrlConfig_S *)arg);
				break;
			case IO_IOG_DBG_LEVEL : 
				ret = xmcs_get_dbg_level((ushort *)arg) ;
				break ;	
			case IO_IOG_PLOAM_GTC_INFO:
				ret = xmcs_get_ploam_gtc_info((PLOAM_GTC_INFO_T *)arg);
		        break;
			case IO_IOG_DRV_POWER_LEVEL:
				ret = xmcs_get_drv_power_level((PLOAM_POWER_LEVEL_T *)arg);
				break;
#ifdef TCSUPPORT_CPU_EN7521
            case IO_IOG_ALL_TCONT_PACKET_INFO:
				ret = xmcs_get_all_tcont_packet_info((ALL_TCONT_COUNTERS_T *)arg);
				break;
            case IO_IOG_TCONT_PACKET_INFO:
				ret = xmcs_get_tcont_packet_info((TCONT_COUNTERS_T *)arg);
				break;
#endif 
#ifdef TCSUPPORT_WAN_GPON
			case IF_IOG_GEMPORT_INFO : 
				ret = xmcs_get_gem_port_info((struct XMCS_GemPortInfo_S *)arg) ;
				break ;
			case IF_IOG_TCONT_INFO : 
				ret = xmcs_get_tcont_info((struct XMCS_TcontInfo_S *)arg) ;
				break ;
#endif /* TCSUPPORT_WAN_GPON */
#ifdef TCSUPPORT_WAN_EPON
			case IF_IOG_LLID_INFO : 
				ret = xmcs_get_llid_info((struct XMCS_EponLlidInfo_S *)arg) ;
				break ;
#endif /* TCSUPPORT_WAN_EPON */
			default:
				PON_MSG(MSG_ERR, "No such I/O command, cmd: %x\n", cmd) ;
				break ;	
		}
	} else {
		PON_MSG(MSG_ERR, "No such I/O command, cmd: %x\n", cmd) ;
	}

	return ret ;
}


