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
#include <ecnt_hook/ecnt_hook_fe.h>

#include "xmcs/xmcs_phy.h"
#include "common/phy_if_wrapper.h"

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

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API) || defined(TCSUPPORT_XPON_HAL_API_EXT)
int g_omci_creat_tcont = 0;
#endif/*TCSUPPORT_COMPILE*/
#ifdef TCSUPPORT_CPU_ARMV8
extern int get_ethaddr(unsigned char *ethaddr, int len);
#endif

extern int isNeedResetTransciver;
extern epon_t eponDrv;
extern int rogueOnuDisableTxPowerFlag;
extern Omci_Oam_Monitor_t * gpMonitor;

 extern void xpon_stop_timer(void);
 extern int eponMpcpRegisterDone(void);

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
	QDMA_TxQosWeightType_t qdmaWeightType = 0;
	QDMA_TxQosWeightScale_t qdmaWeightScale = 0;
	
	if(pWeightCfg->weightType == XMCS_IF_WEIGHT_TYPE_PACKET) {
		qdmaWeightType = QDMA_TXQOS_WEIGHT_BY_PACKET ;
	} else if(pWeightCfg->weightType == XMCS_IF_WEIGHT_TYPE_BYTE) {
		qdmaWeightType = QDMA_TXQOS_WEIGHT_BY_BYTE ;
	} else {
		return -EINVAL ;
	}
	
	if(pWeightCfg->weightType == XMCS_IF_WEIGHT_TYPE_BYTE) {
#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7528)
		if(pWeightCfg->weightScale == XMCS_IF_WEIGHT_SCALE_1B) {
			qdmaWeightScale = QDMA_TXQOS_WEIGHT_SCALE_1B ;
#else
		if(pWeightCfg->weightScale == XMCS_IF_WEIGHT_SCALE_64B) {
			qdmaWeightScale = QDMA_TXQOS_WEIGHT_SCALE_64B ;
#endif
		} else if(pWeightCfg->weightScale == XMCS_IF_WEIGHT_SCALE_16B) {
			qdmaWeightScale = QDMA_TXQOS_WEIGHT_SCALE_16B ;
		} else {
			return -EINVAL ;
		}
	}

	return QDMA_API_SET_TX_QOS_WEIGHT(ECNT_QDMA_WAN, qdmaWeightType, qdmaWeightScale) ;
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

	ret = QDMA_API_GET_TX_QOS_WEIGHT(ECNT_QDMA_WAN, &qdmaWeightType, &qdmaWeightScale) ;
	if(ret != 0) {
		return ret ;
	}

	pWeightCfg->weightType = (qdmaWeightType==QDMA_TXQOS_WEIGHT_BY_PACKET) ? XMCS_IF_WEIGHT_TYPE_PACKET : XMCS_IF_WEIGHT_TYPE_BYTE ;
#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7528)
    pWeightCfg->weightScale = (qdmaWeightScale==QDMA_TXQOS_WEIGHT_SCALE_1B) ? XMCS_IF_WEIGHT_SCALE_1B : XMCS_IF_WEIGHT_SCALE_16B ;
#else
	pWeightCfg->weightScale = (qdmaWeightScale==QDMA_TXQOS_WEIGHT_SCALE_64B) ? XMCS_IF_WEIGHT_SCALE_64B : XMCS_IF_WEIGHT_SCALE_16B ;
#endif
		
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
	
	if(pScheduler->channel>=XPON_CHANNEL_NUMBER) {
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
	
	return QDMA_API_SET_TX_QOS(ECNT_QDMA_WAN, &qosScheduler);
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
static int xmcs_get_channel_scheduler(struct XMCS_ChannelQoS_S *pScheduler)
{
	QDMA_TxQosScheduler_T qosScheduler ;
	int i, ret=0 ;
	
	if(pScheduler->channel>=GPON_TCONT_MAX_NUM) {
		return -EINVAL ;
	}
	qosScheduler.channel = pScheduler->channel ;
	
	ret = QDMA_API_GET_TX_QOS(ECNT_QDMA_WAN, &qosScheduler);
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

	//todo: not implement yet
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
	QDMA_API_SET_TXQ_DEI_DROP_MODE(ECNT_QDMA_WAN, mode);
	
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

	if (pCongest->congestMode.threshold != XPON_OTHER){
		mode = (pCongest->congestMode.threshold==XPON_ENABLE) ? QDMA_ENABLE : QDMA_DISABLE ;	
		//todo: not implement yet

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

	//todo: not implement yet
	return 0;
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
static int xmcs_set_congest_threshold(struct XMCS_TxQueueCongestion_S *pCongest)
{
	//todo: not implement yet
	return 0;
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
	//todo: not implement yet

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
static int xmcs_set_tx_trtcm_scale(struct XMCS_TxTrtcmScale_S *pScale)
{
    //todo: not implement yet

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
	QDMA_TrtcmScale_t qdmaTrtcmScale = 0;
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
	
	if(pTrtcmParams->tsIdx>=XPON_SHAPING_NUMBER) {
		return -EINVAL ;
	}
	
	qdmaTrtcmParams.tsid = pTrtcmParams->tsIdx ;
	qdmaTrtcmParams.cirParamValue = pTrtcmParams->cirValue ;
	qdmaTrtcmParams.cbsParamValue = pTrtcmParams->cbsUnit ;
	qdmaTrtcmParams.pirParamValue = pTrtcmParams->pirValue ;
	qdmaTrtcmParams.pbsParamValue = pTrtcmParams->pbsUnit ;

	//todo: not implement yet
	return 0;
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
	//todo: not implement yet
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
	
	//todo: not implement yet
	return 0;
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
	PSE_PcpType_t psePcpType = 0;

	if(pPcpCfg->pcpType == XMCS_IF_PCP_TYPE_CDM_TX) {
		psePcpType = PSE_PCP_TYPE_CDM_TX ;
	} else if(pPcpCfg->pcpType == XMCS_IF_PCP_TYPE_CDM_RX) {
		psePcpType = PSE_PCP_TYPE_CDM_RX ;
	} else if(pPcpCfg->pcpType == XMCS_IF_PCP_TYPE_GDM_RX) {
		psePcpType = PSE_PCP_TYPE_GDM_RX ;
	} else {
		return -EINVAL ;
	}

	//todo: not implement yet
	
	return 0 ;
}

void prepare_epon(preXponMode_t mode){
	Xpon_Phy_Mode_t current_mode = XPON_PHY_GET(PON_GET_PHY_MODE);
    /* Set the SCU register to select the WAN mode (EPON) */
	if(!(isEN7580 || isEN7581 || isAN7583)){
		XPON_PHY_FW_READY_DISABLE();
		select_xpon_wan(PHY_EPON_CONFIG);
		if(current_mode != PHY_EPON_CONFIG){
			if(XPON_AUTO_MODE == mode){
				XPON_PHY_SET(PON_SET_PHY_SCU_RST);
			}
			XPON_PHY_SET_MODE(PHY_EPON_CONFIG);
		}
		XPON_PHY_FW_READY_ENABLE();
	}
    //clear p6 egress traffic limit
    WRITE_FE_REG(0xbfb59640,0x0);
}

int xmcs_set_connection_start(XPON_Mode_t mode)
{
	if(rogueOnuDisableTxPowerFlag){
		return 0;
	}
	
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_C5_HEN_SFU)
  static unchar first_reset_laser = 0 ;
  unchar writeBuf[6+1] = {0x53,0x60,0x67,0x73,0x4,0x2};
#endif/*TCSUPPORT_COMPILE*/

    XPON_DPRINT_MSG("mode=%d\n", mode);
    /* notify XPON PHY Driver get ready to handle interrupts*/

	if(TRUE == gpPonSysData->fe_reset_flag){
		gpPonSysData->fe_reset_flag = FALSE;
	}else{
    	/*Clear silence!*/
		gpGponPriv->gpon_silence = 0;
		/*Stop silence!*/
		GPON_STOP_TIMER(gpGponPriv->silence_timer) ;
	}

    if(XPON_ENABLE == mode){
		printk("ponTime:Connection Start\n");
		printk("working_mode: %d\n", gpPhyData->working_mode);
		if(!(isEN7580 || isEN7581 || isAN7583)){
			XPON_PHY_SET(PON_SET_PHY_START) ;
			gpPonSysData->sysStartup = PON_WAN_START ;
			if(PHY_GPON_CONFIG==gpPhyData->working_mode){
				if((1==gpMcsPriv->ctrlFlag.report_init_O1)||        \
			   (PHY_LOS_HAPPEN == XPON_PHY_GET(PON_GET_PHY_LOS_STATUS))){      
				xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_STATE_CHANGE, GPON_STATE_O1);
				}
			}
		}else{
			if(PHY_GPON_CONFIG==gpPhyData->working_mode){
				if(1==gpMcsPriv->ctrlFlag.report_init_O1) { 
					xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_STATE_CHANGE, GPON_STATE_O1);
				}
			}
		}
		if(PHY_UNKNOWN_CONFIG == gpPhyData->working_mode){
            start_phy_mode_auto_detection();
        }else{
            XPON_DPRINT_MSG("gpPhyData->working_mode: %d\n", gpPhyData->working_mode);
            switch(gpPhyData->working_mode)
            {
                case PHY_GPON_CONFIG:
				case PHY_GPON_SYM_CONFIG:
                    prepare_gpon(XPON_FIX_MODE);
                    break;

                case PHY_EPON_CONFIG:
                    prepare_epon(XPON_FIX_MODE);
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
		if(isEN7580 || isEN7581 || isAN7583){
			gpPonSysData->sysStartup = PON_WAN_START ;
			XPON_PHY_SET(PON_SET_PHY_START) ;	
		}
		XPON_START_TIMER(gpPhyData->trans_status_refresh_timer,1000) ; /* 1,000 ms */
	}else if((mode == XPON_DISABLE) || (mode ==XPON_POWER_DOWN) ) {
            stop_phy_mode_detect();
            XPON_PHY_SET(PON_SET_PHY_STOP) ;
            gpPonSysData->sysStartup = PON_WAN_STOP ;

            switch(gpPhyData->working_mode)
            {
                case PHY_GPON_CONFIG:
				case PHY_GPON_SYM_CONFIG:
					XPON_DPRINT_MSG("call gpon disable.\n");
        			gpon_disable() ;
                    break;

                case PHY_EPON_CONFIG:
			        eponStop() ;
					XPON_PHY_SET(PON_SET_PHY_START);
					XPON_PHY_FW_READY_ENABLE();
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
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_C5_HEN_SFU)
		printk("isNeedResetTransciver == %d\n",isNeedResetTransciver);
		if( !first_reset_laser && isNeedResetTransciver)
		{
			SIF_X_Write(0,0xc7,0x51,1,0x7B,&writeBuf[0],1);
			SIF_X_Write(0,0xc7,0x51,1,0x7C,&writeBuf[1],1);
			SIF_X_Write(0,0xc7,0x51,1,0x7D,&writeBuf[2],1);
			SIF_X_Write(0,0xc7,0x51,1,0x7E,&writeBuf[3],1);
			SIF_X_Write(0,0xc7,0x51,1,0x7F,&writeBuf[4],1);
			SIF_X_Write(0,0xc7,0x51,1,0x97,&writeBuf[5],1);
			first_reset_laser++ ;
	 	}
#endif/*TCSUPPORT_COMPILE*/
    }

	return 0 ;
}

/******************************************************************************
******************************************************************************/
int xmcs_get_onu_type(XMCSIF_OnuType_t *type)
{
	if(NULL == type)
		return -EINVAL ;

	#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
	*type = XMCS_IF_ONU_TYPE_HGU;
	#else/*TCSUPPORT_COMPILE*/
	if(gpPonSysData->sysOnuType == PON_ONU_TYPE_HGU) {
		*type = XMCS_IF_ONU_TYPE_HGU;
	} else if(gpPonSysData->sysOnuType == PON_ONU_TYPE_SFU) {
		*type = XMCS_IF_ONU_TYPE_SFU;
	} else {
		*type = XMCS_IF_ONU_TYPE_UNKNOWN;
	} 
	#endif/*TCSUPPORT_COMPILE*/
	return 0;
}

/*
______________________________________________________________________________
**	function name
**		xmcs_get_onu_mac
**	description:
**		read onu mac address from flash.
**	parameters:
**		onuMac:
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________
*/
int xmcs_get_onu_mac(XMCSIF_OnuMac_t *onuMac){
	unsigned char devMac[EPON_MAC_ADDRESS_LENTH] = {0};
#ifndef TCSUPPORT_CPU_ARMV8
	int i = 0;
#endif
	if(NULL == onuMac)
		return -EINVAL ;

    #ifdef TCSUPPORT_CPU_ARMV8
    if(get_ethaddr(devMac, sizeof(devMac)) != 0){
        printk("func:%s get ethaddr error!!!\n",__func__);
    }
    #else
	for (i = 0; i < ETH_ADDR_LEN; i++)
		devMac[i] = READ_FLASH_BYTE(flash_base + EPON_MAC_ADDRESS_OFFSET + i);
    #endif
    
    memcpy(onuMac->mac, devMac, sizeof(devMac));
	
	return 0;
}

/*
______________________________________________________________________________
**	function name
**		xmcs_get_onu_mode_from_flash
**	description:
**		this api is used to determine whether to read onu mode from flash.
**		752x: onu mode is read from romfile.cfg, not flash,
**	parameters:
**		isFromFlash, if 752x, isFromFlash = 0 ,if 7580, isFromFlash = 1;
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________
*/
int xmcs_get_onu_mode_from_flash(unsigned char *isFromFlash){
	unsigned char flashFlag = 0;
	
	if(NULL == isFromFlash)
		return -EINVAL ;
	
	if(isEN7580 || isEN7581 || isAN7583)
		flashFlag = 1;
	else
		flashFlag = 0;

	if(copy_to_user(isFromFlash, &flashFlag, sizeof(unsigned char))){
			printk("func:%s calling copy_to_user fail \n",__FUNCTION__);
			return -EINVAL;
	}

	return 0;
}

/*
______________________________________________________________________________
**	function name
**		xmcs_clear_rogue_status
**	description:
**		This is clear onu rogue status api function.
**	parameters:
**		null:
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________
*/
int xmcs_clear_rogue_status(void)
{
	gpPonSysData->ponRogueStatus=PON_STATUS_NORMAL;

	return 0;
}

/*
______________________________________________________________________________
**	function name
**		xmcs_get_rogue_status
**	description:
**		This is get onu rogue status api function.
**	parameters:
**		status:return PON_RogueStatus_t
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________
*/
int xmcs_get_rogue_status(PON_RogueStatus_t *status)
{
	if(NULL == status)
		return -EINVAL ;
	*status = gpPonSysData->ponRogueStatus;

	return 0;
}

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

		case XMCS_IF_WAN_DETECT_MODE_GPON_SYM:
			gpPhyData->is_fix_mode	= TRUE;
			gpPhyData->working_mode = PHY_GPON_SYM_CONFIG;
			break;
			
        case XMCS_IF_WAN_DETECT_MODE_EPON:
            gpPhyData->is_fix_mode  = TRUE;
            gpPhyData->working_mode = PHY_EPON_CONFIG;
            break;

        case XMCS_IF_WAN_DETECT_MODE_AUTO:
            gpPhyData->is_fix_mode  = FALSE;
			gpPhyData->working_mode  = PHY_UNKNOWN_CONFIG;
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
                if(PHY_GPON_CONFIG == gpPhyData->working_mode) {
        	pSysLinkStatus->detectMode = XMCS_IF_WAN_DETECT_MODE_GPON;
		} else if(PHY_GPON_SYM_CONFIG == gpPhyData->working_mode) {
        	pSysLinkStatus->detectMode = XMCS_IF_WAN_DETECT_MODE_GPON_SYM;
		} else {
			pSysLinkStatus->detectMode = XMCS_IF_WAN_DETECT_MODE_EPON;
		}
        
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

	return 0 ;
}

/******************************************************************************
** 				mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01   
******************************************************************************/
int xmcs_set_wan_cnt_stats(uint mask)
{
	if(mask&0x01)
		FE_API_SET_CLEAR_MIB(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX);
	else if(mask&0x02)
		FE_API_SET_CLEAR_MIB(FE_GDM_SEL_GDMA2, FE_GDM_SEL_RX);

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
    FE_TxCnt_t txCnt = {0};
    FE_RxCnt_t rxCnt = {0};

	PHY_FecCount_T phyFecCount = {0};
	struct XMCS_WanCntStats_S wan_cnt_temp = {0};
	
	if (pWanCntStats == NULL)
		return -EINVAL;

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

    if(rdk_gtc_dbg.proc_flag) {
        wan_cnt_temp.rxHecErrorCnt = rdk_gtc_dbg.hecCounter.XGEMHECErrCount ;
        rdk_gtc_dbg.proc_flag = 0;
    }
#endif

    
    XPON_PHY_GET_API(PON_GET_PHY_RX_FEC_COUNTER, &phyFecCount);
	wan_cnt_temp.rxFecErrorCnt = phyFecCount.uncorrect_codewords;
	wan_cnt_temp.rxFecCerrorCnt= phyFecCount.correct_codewords;
    wan_cnt_temp.FecSeconds    = phyFecCount.fec_seconds;
	wan_cnt_temp.BipError      = XPON_PHY_GET(PON_GET_PHY_BIP_COUNTER);

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

	wan_cnt_temp.rxFCDropCnt 		= rxCnt.rxFCDropCnt;
	wan_cnt_temp.rxRCDropCnt 		= rxCnt.rxRCDropCnt;
	wan_cnt_temp.rxOVDropCnt 		= rxCnt.rxOVDropCnt;
	wan_cnt_temp.rxERRDropCnt 		= rxCnt.rxERRDropCnt;
	wan_cnt_temp.rxOKPktCnt 		= rxCnt.rxOKPktCnt;
	wan_cnt_temp.rxOKByteCnt 		= rxCnt.rxOKByteCnt;
	wan_cnt_temp.rxOversizeCnt 		= rxCnt.rxOversizeCnt;
	wan_cnt_temp.rxUnderSizeCnt 	= rxCnt.rxUnderSizeCnt;

	if(0 != copy_to_user((void __user *)pWanCntStats, &wan_cnt_temp, sizeof(wan_cnt_temp) ))
	{
	    printk("copy_to_user failed\n");
		return -1;
	}
	return 0;
}

/******************************************************************************
******************************************************************************/
int xmcs_get_ploam_gtc_info(PLOAM_GTC_INFO_T * gtc_info)
{
    PLOAM_GTC_INFO_T temp = {0};
    REG_G_PLOu_GUARD_BIT gponGuardBit;
	REG_G_PLOu_PRMBL_TYPE1_2 gponPrmblType1_2;
	REG_G_PLOu_PRMBL_TYPE3 gponPrmblType3;
	uint regDelimiter = 0; 
	int ret = -1;

    if(gtc_info == NULL)
		return -1;
		
	gponGuardBit.Raw = 0;
	gponPrmblType1_2.Raw = 0;
	gponPrmblType1_2.Raw = 0;
	gponPrmblType3.Raw = 0;
	memset(&temp, 0, sizeof(PLOAM_GTC_INFO_T));
    memset(&gponGuardBit, 0, sizeof(REG_G_PLOu_GUARD_BIT));
    memset(&gponPrmblType1_2, 0, sizeof(REG_G_PLOu_PRMBL_TYPE1_2));
    memset(&gponPrmblType3, 0, sizeof(REG_G_PLOu_PRMBL_TYPE3));
    
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

int xmcs_get_all_tcont_packet_info(ALL_TCONT_COUNTERS_T * pallcounters)
{
    int i;
	ALL_TCONT_COUNTERS_T temp;
	struct XMCS_TcontInfo_S tcontInfo;
	int ret = -1;

    if(pallcounters == NULL)
		return -1;

	memset(&temp, 0, sizeof(ALL_TCONT_COUNTERS_T));
    memset(&tcontInfo, 0, sizeof(struct XMCS_TcontInfo_S));
    
	xmcs_get_tcont_info(&tcontInfo);
    for(i = 0; i < tcontInfo.entryNum; i++)
    {
		ret = QDMA_API_DUMP_CNTR_CHANNEL(ECNT_QDMA_WAN, tcontInfo.info[i].channel);
		if(ret == -1)
		{
		    printk("QDMA_API_DUMP_CNTR_CHANNEL channel_id %d return value error\n", i);
			continue;
		}

		temp.counters[i].tcont = tcontInfo.info[i].channel;
		temp.counters[i].alloc_id = tcontInfo.info[i].allocId;
		temp.counters[i].tx_counter = ret;		
    }
	temp.tcont_num = tcontInfo.entryNum;
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
	struct XMCS_TcontInfo_S tcontInfo;
	
    if(pcounters == NULL)
		return -1;
    ret= copy_from_user(&temp, pcounters, sizeof(TCONT_COUNTERS_T));
	if(0 != ret)
	{
	    printk("copy_from_user failed\n");
		return -1;
	} 

	memset(&tcontInfo, 0, sizeof(struct XMCS_TcontInfo_S));
	xmcs_get_tcont_info(&tcontInfo);
    for(i = 0; i < tcontInfo.entryNum; i++)
	{
        if(tcontInfo.info[i].allocId == temp.alloc_id)
        {
			break;
        }
    }

	ret = QDMA_API_DUMP_CNTR_CHANNEL(ECNT_QDMA_WAN, tcontInfo.info[i].channel);
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


/*****************************************************************************
******************************************************************************/
static int xmcs_get_pon_online_duration(ulong *arg)
{
    unsigned long duration=0;
    
    if( (GPON_TRAFFIC_UP == gwan_get_traffic_status()) \
        ||(MPCP_STATE_REGISTERED == eponMpcpRegisterDone()))
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

int xpon_reset_qdma_tx_buf(void ){
	QDMA_TxBufCtrl_T txBufCtrl;

	txBufCtrl.mode = QDMA_ENABLE;

	if(isEN7581 || isAN7583){
		 if(gpWanPriv->activeChannelNum <=8){
	    	txBufCtrl.chnThreshold = 0x40;
	    	txBufCtrl.totalThreshold = (0x20+0x10-1)*(gpWanPriv->activeChannelNum)*2;
		}else{
			txBufCtrl.chnThreshold = 0x10;
			txBufCtrl.totalThreshold = ((0x08+0x10-1)*(gpWanPriv->activeChannelNum-1) + 1)*2;
		}
	}else if(isEN7523 || isAN7552){
		 if(gpWanPriv->activeChannelNum <=8){
	    	txBufCtrl.chnThreshold = 0x20;
	    	txBufCtrl.totalThreshold = (0x20+0x10-1)*(gpWanPriv->activeChannelNum);
		}else{
			txBufCtrl.chnThreshold = 0x08;
			txBufCtrl.totalThreshold = (0x08+0x10-1)*(gpWanPriv->activeChannelNum-1) + 1;
		}
	}else if (isEN751221 && (gpWanPriv->activeChannelNum >=16)){
		txBufCtrl.chnThreshold = 4;
		txBufCtrl.totalThreshold = 0xe0;
	}else if(isEN7528){
	/* GDMP SRAM 32KB used in pse for IQ REV5*/
        if(gpWanPriv->activeChannelNum >=8){
        	txBufCtrl.chnThreshold = min(0x08, 0xE0/(uint8)gpWanPriv->activeChannelNum);
        	txBufCtrl.totalThreshold = 0xE0;
        }else{
        	txBufCtrl.chnThreshold = min(0x20, 0xE0/(uint8)gpWanPriv->activeChannelNum);
        	txBufCtrl.totalThreshold = 0xE0;
        }
	}else{
		txBufCtrl.chnThreshold = min(0x20, 0x80/(uint8)gpWanPriv->activeChannelNum);
		txBufCtrl.totalThreshold = 0x80;
	}

		QDMA_API_SET_TXBUF_THRESHOLD(ECNT_QDMA_WAN, &txBufCtrl);

	return 0;
}

int xpon_set_qos(u8 enable, u8 queueNum){
	
	if (enable == DISABLE) {
		gpWanPriv->devCfg.flags.isQosUp = XPON_DISABLE;
	} else if (enable == ENABLE){
		gpWanPriv->devCfg.flags.isQosUp = XPON_ENABLE;
		if ((queueNum < 9) && (queueNum > 4)) {
			gpWanPriv->greenMaxthreshold = 64/queueNum;
		}	
	} else {
		printk("xpon_set_qos:wrong value\n");
	}

	return 0;
}

int mulitcast_ani = -1;

#ifdef TCSUPPORT_WAN_GPON
/***************************************************************
***************************************************************/
static int xmcs_assign_gem_port(struct XMCS_GemPortAssign_S *pGemAssign)
{
	int i, ret ;
	unchar gemIdx;
	ushort gemPortId;
	if(pGemAssign==NULL || pGemAssign->entryNum>=GPON_GEMPORT_MAX_NUM) {
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
		if(gemPortId < GPON_MAX_GEM_ID){
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
	
	if(pGemAssign==NULL || pGemAssign->entryNum>=GPON_GEMPORT_MAX_NUM) {
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
	unchar gemIdx, channel=GPON_TCONT_MAX_NUM ;
	int i ;
	int ret = 0;
	unchar encryption = 0;
	
	if(pGemCreate->gemPortId>=GPON_MAX_GEM_ID || pGemCreate->gemPortId==GPON_OMCC_ID) {
		return -EINVAL ;
	}
	
	if(pGemCreate->gemType == GPON_UNICAST_GEM) {
		
		if(pGemCreate->allocId>=GPON_MAX_ALLOC_ID) {
			return -EINVAL ;
		}
		
		for(i=0 ; i<GPON_TCONT_MAX_NUM ; i++) {
			if(gpWanPriv->gpon.allocId[i] == pGemCreate->allocId) {
				channel = i ;				
				break ;
			}
		}

		if(i == GPON_TCONT_MAX_NUM){
			channel = GPON_UNKNOWN_CHANNEL;
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
		ret = gwan_create_new_gemport(pGemCreate->gemPortId, channel, encryption,pGemCreate->allocId);

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT) || defined(TCSUPPORT_FWC_ENV) || defined(TCSUPPORT_FH_PON)
	if(0 == ret && gpWanPriv->gpon.gemPort[gemIdx].info.ani != 1)
	{
		printk("Fh creat gemport %d, add assign \n",pGemCreate->gemPortId);
		assignGemportId(pGemCreate->gemPortId);
	}
#endif/*TCSUPPORT_COMPILE*/	

	}
	return ret;
}

/***************************************************************
***************************************************************/
int xmcs_remove_gem_port(ushort gemPortId)
{
    if(gemPortId>=GPON_MAX_GEM_ID || gemPortId==GPON_OMCC_ID){
        return -EINVAL ;
    }
    if(GPON_OMCC_ID != gemPortId){
        gpon_recover_delete_gemport(gemPortId);
    }

	return gwan_remove_gemport(gemPortId) ;
}

/***************************************************************
***************************************************************/
int xmcs_set_gpon_silence(uint flag)
{
	if(flag == 1){
		gpon_start_silence() ;
	}else{
		gpon_stop_silence() ;
	}
	return 0;
}

/***************************************************************
***************************************************************/
int xmcs_remove_all_gem_port(void)
{
	return gwan_remove_all_gemport() ;
}

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
	for(i=0 ; i<GPON_GEMPORT_MAX_NUM ; i++) {
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
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
int xmcs_set_qos_policer_creat(struct XMCS_QosPolicerCreat_S *pQosPolicer)
{
	int cir = 0;
	QDMA_TxQueueTrtcm_T txqTrtcm;

	printk("qos_policer_creat\n");

    if(!isTotalBWSet)
    {
    	qosFlag = QOS_HW_CAR;
		trtcmEnable = 1;
	    //todo: not implement yet
	    
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

	//todo: not implement yet

	if(0 == gQueueMask)
	{
		//todo: not implement yet
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

	//todo: not implement yet

	gQueueMask &= (63 - (1<<(pQosPolicerDelete->policer_id-1)));

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
int xmcs_check_allocid_exist(ushort allocId)
{
	PON_MSG(MSG_TRACE, "[%s][%d]went in func!the allocId is %d\n",__FUNCTION__,__LINE__,allocId) ;
	if(allocId != gpGponPriv->gponCfg.onu_id)
	{
		if(TRUE == gwanCheckAllocIdExist(allocId))
		{
			PON_MSG(MSG_TRACE, "[%s][%d]gwan has this allocId,return 0!\n",__FUNCTION__,__LINE__) ;
			return 0;
		}
		else
		{
			PON_MSG(MSG_ERR, "[%s][%d]gwanCheckAllocIdExist FAIL,return -1!\n",__FUNCTION__,__LINE__) ;
			return -1;
		}
	}
	PON_MSG(MSG_TRACE, "[%s][%d]the current alloc id is onu id ,return 0\n",__FUNCTION__,__LINE__) ;
	return 0;
}

int xmcs_get_tcont_info(struct XMCS_TcontInfo_S *pTcontInfo)
{
	int i ;
	
	pTcontInfo->entryNum = 0 ;
	for(i=0 ; i<GPON_TCONT_MAX_NUM ; i++) {
		if(gpWanPriv->gpon.allocId[i] != GPON_UNASSIGN_ALLOC_ID) {
			pTcontInfo->info[pTcontInfo->entryNum].allocId = gpWanPriv->gpon.allocId[i] ;
			pTcontInfo->info[pTcontInfo->entryNum].channel = i ;

			pTcontInfo->entryNum++ ;
		}
	}
	
	return 0 ;
}

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


/***************************************************************
***************************************************************/
int xmcs_create_tcont_info(struct XMCS_TcontCfg_S *pTcontCfg)
{
	ushort allocId = pTcontCfg->allocId; 
	
	if(allocId != gpGponPriv->gponCfg.onu_id)
	{
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API) || defined(TCSUPPORT_XPON_HAL_API_EXT)
		g_omci_creat_tcont  = 1;
#endif/*TCSUPPORT_COMPILE*/

		//xmcs_remove_tcont_info(allocId);
		if(TRUE == gwanCheckAllocIdExist(allocId))
		{
			return 0;
		}
		if (0 ==gwan_create_new_tcont(allocId))
		{
			gpWanPriv->activeChannelNum ++;
			if (gpWanPriv->activeChannelNum > GPON_TCONT_MAX_NUM)
			{
				gpWanPriv->activeChannelNum = GPON_TCONT_MAX_NUM;
			}
            if(gpMonitor->run_state == MONITOR_DONE){
			    xpon_reset_qdma_tx_buf();
            }
		}
	}
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API) || defined(TCSUPPORT_XPON_HAL_API_EXT)	
	g_omci_creat_tcont = 0;
#endif/*TCSUPPORT_COMPILE*/		
	return 0 ;
}

/***************************************************************
***************************************************************/
int xmcs_get_omcc_info(struct XMCS_OMCC_Info_S *ptOmccInfo)
{
    if(NULL == ptOmccInfo)
    {
        return -EINVAL ;
    }
    
    ptOmccInfo->allocId   = GPON_ONU_ID;
    ptOmccInfo->gemportid = GPON_OMCC_ID;

	return 0 ;
}
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API) || defined(TCSUPPORT_XPON_HAL_API_EXT)


#endif/*TCSUPPORT_COMPILE*/

#endif /* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
/***************************************************************
***************************************************************/
int xmcs_create_llid(struct XMCS_EponLlidCreate_S *pLlidCreate)
{
	unchar idx = pLlidCreate->idx ;
	
	if(idx >= EPON_1G_MAX_LLID_NUM) {
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

/***************************************************************
***************************************************************/
int xmcs_remove_llid(unchar idx)
{    
    eponLlid_t *llidEntry_p = NULL;
    eponMpcpDiscFsm_t *mpcpDiscFsm = NULL;
    
	if(idx >= EPON_1G_MAX_LLID_NUM) {
		return -EINVAL ;
	}

	llidEntry_p = &(eponDrv.eponLlidEntry[idx]);
	mpcpDiscFsm = &(llidEntry_p->eponMpcp.eponDiscFsm);

	if(mpcpDiscFsm->mpcpDiscState != MPCP_STATE_DENIED){
	mpcpDiscFsm->mpcpDiscState =  MPCP_STATE_WAIT;
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

/***************************************************************
***************************************************************/
static int xmcs_set_epon_rx_config(struct XMCS_EponRxConfig_S *pRxCfg)
{
	unchar idx = pRxCfg->idx ;
	
	if(idx >= EPON_1G_MAX_LLID_NUM) {
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
	
	if(idx >= EPON_1G_MAX_LLID_NUM) {
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
	for(i=0 ; i<EPON_1G_MAX_LLID_NUM ; i++) {
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


/***************************************************************
***************************************************************/
int xmcs_set_rogue_state(unchar rogue_mode)
{
	if(PHY_SUCCESS != XPON_PHY_SET_ROGUE(rogue_mode) ) {
		return -1 ;
	}
	if(XPON_ROUGE_STATE_TRUE == rogue_mode){
		xpon_phy_start_rouge_handle();
	}else{
		xpon_phy_stop_rouge_handle();
	}
	return 0;
}



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
	
#ifdef TCSUPPORT_CPU_ARMV8_64
		cmd = cmd & IOCTL_CMD;
#endif
	
	if(_IOC_DIR(cmd) == _IOC_WRITE) {
		switch(cmd) {
			case IF_IOS_QOS_WEIGHT_CONFIG :
                {
                    struct XMCS_QoSWeightConfig_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_QoSWeightConfig_S *)arg,sizeof(struct XMCS_QoSWeightConfig_S),ret);
				    ret = xmcs_set_qos_weight_config(&temp) ;
				}
				break ;	
			case IF_IOS_QOS_SCHEDULER :
                {
                    struct XMCS_ChannelQoS_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_ChannelQoS_S *)arg,sizeof(struct XMCS_ChannelQoS_S),ret);
                    ret = xmcs_set_channel_scheduler(&temp) ;
			    }
				break ;	
			case IF_IOS_TRTCM_CONGEST_MODE :
                {
                    struct XMCS_TxQueueCongestion_S temp;
                    memset(&temp, 0 , sizeof(struct XMCS_TxQueueCongestion_S));
                    COPY_FROM_USER(&temp,(struct XMCS_TxQueueCongestion_S *)arg,sizeof(struct XMCS_TxQueueCongestion_S),ret);
                    ret = xmcs_set_trtcm_congest_mode(&temp) ;
				}
				break ;	
			case IF_IOS_DEI_CONGEST_MODE :
                {
                    struct XMCS_TxQueueCongestion_S temp;
                    memset(&temp, 0 , sizeof(struct XMCS_TxQueueCongestion_S));
                    COPY_FROM_USER(&temp,(struct XMCS_TxQueueCongestion_S *)arg,sizeof(struct XMCS_TxQueueCongestion_S),ret);
				    ret = xmcs_set_dei_congest_mode(&temp) ;
				}
				break ;	
			case IF_IOS_THRESHOLD_CONGEST_MODE :
                {
                    struct XMCS_TxQueueCongestion_S temp;
                    memset(&temp, 0 , sizeof(struct XMCS_TxQueueCongestion_S));
                    COPY_FROM_USER(&temp,(struct XMCS_TxQueueCongestion_S *)arg,sizeof(struct XMCS_TxQueueCongestion_S),ret);
				    ret = xmcs_set_threshold_congest_mode(&temp) ;
				}
				break ;	
			case IF_IOS_CONGEST_SCALE :
                {
                    struct XMCS_TxQueueCongestion_S temp;
                    memset(&temp, 0 , sizeof(struct XMCS_TxQueueCongestion_S));
                    COPY_FROM_USER(&temp,(struct XMCS_TxQueueCongestion_S *)arg,sizeof(struct XMCS_TxQueueCongestion_S),ret);
				    ret = xmcs_set_congest_scale(&temp) ;
				}
				break ;
			case IF_IOS_ROGUE_MODE :
				ret = xmcs_set_rogue_state((unchar)arg);
				break;
			case IF_IOS_CONGEST_THRESHOLD :
                {
                    struct XMCS_TxQueueCongestion_S temp;
                    memset(&temp, 0 , sizeof(struct XMCS_TxQueueCongestion_S));
                    COPY_FROM_USER(&temp,(struct XMCS_TxQueueCongestion_S *)arg,sizeof(struct XMCS_TxQueueCongestion_S),ret);
					ret = xmcs_set_congest_threshold(&temp) ;
				}
				break ;	
			case IO_IOS_TX_TRTCM_SCALE :
                {
                    struct XMCS_TxTrtcmScale_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_TxTrtcmScale_S *)arg,sizeof(struct XMCS_TxTrtcmScale_S),ret);
					ret = xmcs_set_tx_trtcm_scale(&temp) ;
				}
				break ;	
			case IO_IOS_TX_TRTCM_PARAMS :
                {
                    struct XMCS_TxQueueTrtcm_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_TxQueueTrtcm_S *)arg,sizeof(struct XMCS_TxQueueTrtcm_S),ret);
					ret = xmcs_set_tx_trtcm_params(&temp) ;
				}
				break ;	
			case IO_IOS_PCP_CONFIG :
                {
                    struct XMCS_TxPCPConfig_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_TxPCPConfig_S *)arg,sizeof(struct XMCS_TxPCPConfig_S),ret);
					ret = xmcs_set_pcp_config(&temp) ;
				}
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
                {
                    struct XMCS_StormCtrlConfig_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_StormCtrlConfig_S *)arg,sizeof(struct XMCS_StormCtrlConfig_S),ret);
                    ret = xmcs_set_storm_ctrl_config(&temp);
				}
                break;	
			case IO_IOS_DBG_LEVEL : 
                {
                    struct XMCS_DebugLevel_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_DebugLevel_S *)arg,sizeof(struct XMCS_DebugLevel_S),ret);
                    ret = xmcs_set_dbg_level(&temp) ;
				}
				break ;
#ifdef TCSUPPORT_WAN_GPON
			case IF_IOS_GEMPORT_ASSIGN : 
                {
                    struct XMCS_GemPortAssign_S temp;
                    memset(&temp, 0 , sizeof(struct XMCS_GemPortAssign_S));
                    COPY_FROM_USER(&temp,(struct XMCS_GemPortAssign_S *)arg,sizeof(struct XMCS_GemPortAssign_S),ret);
                    ret = xmcs_assign_gem_port(&temp) ;
			    }
				break ;
			case IF_IOS_GEMPORT_UNASSIGN : 
                {
                    struct XMCS_GemPortAssign_S temp;
                    memset(&temp, 0 , sizeof(struct XMCS_GemPortAssign_S));
                    COPY_FROM_USER(&temp,(struct XMCS_GemPortAssign_S *)arg,sizeof(struct XMCS_GemPortAssign_S),ret);
                    ret = xmcs_unassign_gem_port(&temp) ;
                }
				break ;
			case IF_IOS_GEMPORT_CREATE : 
                {
                    struct XMCS_GemPortCreate_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_GemPortCreate_S *)arg,sizeof(struct XMCS_GemPortCreate_S),ret);
                    ret = xmcs_create_gem_port(&temp) ;
				}
				break ;
			case IF_IOS_GEMPORT_REMOVE : 
				{
					ushort temp =0;
					COPY_FROM_USER(&temp,(ushort *)arg,sizeof(ushort),ret);
					ret = xmcs_remove_gem_port(temp) ;
				}
				break ;
			case IF_IOS_GEMPORT_REMOVE_ALL : 
				ret = xmcs_remove_all_gem_port() ;
				break ;
			case IF_IOS_GEMPORT_LOOPBACK :
                {
                    struct XMCS_GemPortLoopback_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_GemPortLoopback_S *)arg,sizeof(struct XMCS_GemPortLoopback_S),ret);
                    ret = xmcs_set_gem_port_loopback(&temp) ;
			    } 
				break ;
			case IF_IOS_TCONT_INFO_REMOVE : 
				ret = xmcs_remove_tcont_info(*(ushort *)arg) ;
				break ;
			case IF_IOS_GPON_SILENCE:
				ret = xmcs_set_gpon_silence((uint)arg);
				break;
			case IF_IOG_ALLOCID_INFO:
				ret = xmcs_check_allocid_exist((ushort)arg);
				break;	
			case IF_IOS_TCONT_INFO_CREATE : 
                {
                    struct XMCS_TcontCfg_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_TcontCfg_S *)arg,sizeof(struct XMCS_TcontCfg_S),ret);
                    ret = xmcs_create_tcont_info(&temp) ;
				}
				break ;
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
            case IO_IOS_QOS_CREAT_POLICER:
                {
                    struct XMCS_QosPolicerCreat_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_QosPolicerCreat_S *)arg,sizeof(struct XMCS_QosPolicerCreat_S),ret);
                    ret = xmcs_set_qos_policer_creat(&temp);
	            }
	        break;	
            case IO_IOS_QOS_DELETE_POLICER:
                {
                    struct XMCS_QosPolicerDelete_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_QosPolicerDelete_S *)arg,sizeof(struct XMCS_QosPolicerDelete_S),ret);
                    ret = xmcs_set_qos_policer_delete(&temp);
	            }
	        break;	
            case IO_IOS_QOS_OVERALL_RATELIMIT_CONFIG:
                {
                    struct XMCS_OverallRatelimitConfig_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_OverallRatelimitConfig_S *)arg,sizeof(struct XMCS_OverallRatelimitConfig_S),ret);
                    ret = xmcs_set_qos_overall_ratelimit(&temp);
			    }
			break;
#endif			
#endif /* TCSUPPORT_WAN_GPON */
#ifdef TCSUPPORT_WAN_EPON
			case IF_IOS_LLID_CREATE : 
                {
                    struct XMCS_EponLlidCreate_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_EponLlidCreate_S *)arg,sizeof(struct XMCS_EponLlidCreate_S),ret);
                    ret = xmcs_create_llid(&temp) ;
				}
				break ;
			case IF_IOS_LLID_REMOVE : 
				ret = xmcs_remove_llid((unchar)arg) ;
				break ;
			case IF_IOS_LLID_RX_CFG : 
                {
                    struct XMCS_EponRxConfig_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_EponRxConfig_S *)arg,sizeof(struct XMCS_EponRxConfig_S),ret);
                    ret = xmcs_set_epon_rx_config(&temp) ;
				}
				break ;
			case IF_IOS_LLID_TX_CFG : 
                {
                    struct XMCS_EponTxConfig_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_EponTxConfig_S *)arg,sizeof(struct XMCS_EponTxConfig_S),ret);
                    ret = xmcs_set_epon_tx_config(&temp) ;
				}
				break ;
#endif /* TCSUPPORT_WAN_EPON */
			default:
				PON_MSG(MSG_ERR, "No such I/O command, cmd: %x\n", cmd) ;
				break ;
		}
	} else if(_IOC_DIR(cmd) == _IOC_READ) {
		switch(cmd) {
			case IF_IOG_QOS_WEIGHT_CONFIG :
                {
                    struct XMCS_QoSWeightConfig_S temp = {0};
				    ret = xmcs_get_qos_weight_config(&temp) ;
                    COPY_TO_USER((struct XMCS_QoSWeightConfig_S *)arg,&temp,sizeof(struct XMCS_QoSWeightConfig_S),ret);
				}
				break ;	
			case IF_IOG_QOS_SCHEDULER :
                {
                    struct XMCS_ChannelQoS_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_ChannelQoS_S *)arg,sizeof(struct XMCS_ChannelQoS_S),ret);
				    ret = xmcs_get_channel_scheduler(&temp) ;
                    COPY_TO_USER((struct XMCS_ChannelQoS_S *)arg,&temp,sizeof(struct XMCS_ChannelQoS_S),ret);
                }
				break ;	
			case IO_IOG_CONGEST_INFO :
                {
                    struct XMCS_TxQueueCongestion_S temp;
                    memset(&temp, 0 , sizeof(struct XMCS_TxQueueCongestion_S));
				    ret = xmcs_get_congest_info(&temp) ;
                    COPY_TO_USER((struct XMCS_TxQueueCongestion_S *)arg,&temp,sizeof(struct XMCS_TxQueueCongestion_S),ret);
                }
				break ;	
			case IO_IOG_TX_TRTCM_SCALE :
                {
                    struct XMCS_TxTrtcmScale_S temp = {0};
				    ret = xmcs_get_tx_trtcm_scale(&temp) ;
                    COPY_TO_USER((struct XMCS_TxTrtcmScale_S *)arg,&temp,sizeof(struct XMCS_TxTrtcmScale_S),ret);
                }
				break ;	
			case IO_IOG_TX_TRTCM_PARAMS :
                {
                    struct XMCS_TxQueueTrtcm_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_TxQueueTrtcm_S *)arg,sizeof(struct XMCS_TxQueueTrtcm_S),ret);
				    ret = xmcs_get_tx_trtcm_params(&temp) ;
                    COPY_TO_USER((struct XMCS_TxQueueTrtcm_S *)arg,&temp,sizeof(struct XMCS_TxQueueTrtcm_S),ret);
                }
				break ;	
			case IO_IOG_PCP_CONFIG :
                {
                    struct XMCS_TxPCPConfig_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_TxPCPConfig_S *)arg,sizeof(struct XMCS_TxPCPConfig_S),ret);
					ret = xmcs_get_pcp_config(&temp) ;
                    COPY_TO_USER((struct XMCS_TxPCPConfig_S *)arg,&temp,sizeof(struct XMCS_TxPCPConfig_S),ret);
                }
				break ;	
			case IO_IOG_WAN_LINK_CONFIG :
                {
                    struct XMCS_WanLinkConfig_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_WanLinkConfig_S *)arg,sizeof(struct XMCS_WanLinkConfig_S),ret);
				    ret = xmcs_get_wan_link_status(&temp) ;
                    COPY_TO_USER((struct XMCS_WanLinkConfig_S *)arg,&temp,sizeof(struct XMCS_WanLinkConfig_S),ret);
                }
				break ;
			case IO_IOG_ONU_TYPE :
                {
                    XMCSIF_OnuType_t temp = XMCS_IF_ONU_TYPE_UNKNOWN;
    				ret = xmcs_get_onu_type(&temp) ;
                    COPY_TO_USER((XMCSIF_OnuType_t *)arg,&temp,sizeof(XMCSIF_OnuType_t),ret);
                }
				break ;
			case IO_IOG_BBF_247_FLAG :
				{
					ret = 0;
					COPY_TO_USER((uint8_t *)arg,&gpPonSysData->sysBBF247, sizeof(uint8_t), ret);
				}
				break ;
			case IO_IOG_ONU_MODE_FROM_FLASH	:
				ret = xmcs_get_onu_mode_from_flash((unsigned char *) arg);
				break;
			case IO_IOS_CLR_ROGUE_STATUS :
				ret = xmcs_clear_rogue_status();
				break ;
			case IO_IOG_GET_ROGUE_STATUS :
				{
					PON_RogueStatus_t temp=0;
					ret = xmcs_get_rogue_status(&temp);
					COPY_TO_USER((PON_RogueStatus_t *)arg,&temp,sizeof(PON_RogueStatus_t),ret);
				}
				break ;
			case IO_IOG_ONU_MAC :
                {
                    XMCSIF_OnuMac_t temp;
                    memset(&temp, 0 , sizeof(XMCSIF_OnuMac_t));
				    ret = xmcs_get_onu_mac(&temp);
                    COPY_TO_USER((XMCSIF_OnuMac_t *)arg,&temp,sizeof(XMCSIF_OnuMac_t),ret)
			    }
				break;
			case IO_IOG_WAN_CNT_STATS:
				ret = xmcs_get_wan_cnt_stats((struct XMCS_WanCntStats_S *)arg);
				break;
			case IO_IOG_STORM_CTL_CONFIG:
                {
                    struct XMCS_StormCtrlConfig_S temp = {0};
                    COPY_FROM_USER(&temp,(struct XMCS_StormCtrlConfig_S *)arg,sizeof(struct XMCS_StormCtrlConfig_S),ret);
                    ret = xmcs_get_storm_ctrl_config(&temp);
                    COPY_TO_USER((struct XMCS_StormCtrlConfig_S *)arg,&temp,sizeof(struct XMCS_StormCtrlConfig_S),ret);
                }
				break;
			case IO_IOG_DBG_LEVEL : 
                {
                    ushort temp = 0;
				    ret = xmcs_get_dbg_level(&temp) ;
                    COPY_TO_USER((ushort *)arg,&temp,sizeof(ushort),ret);
                }
				break ;	
			case IO_IOG_PLOAM_GTC_INFO:
				ret = xmcs_get_ploam_gtc_info((PLOAM_GTC_INFO_T *)arg);
		        break;
			case IO_IOG_DRV_POWER_LEVEL:
				ret = xmcs_get_drv_power_level((PLOAM_POWER_LEVEL_T *)arg);
				break;
            case IO_IOG_ALL_TCONT_PACKET_INFO:
				ret = xmcs_get_all_tcont_packet_info((ALL_TCONT_COUNTERS_T *)arg);
				break;
            case IO_IOG_TCONT_PACKET_INFO:
				ret = xmcs_get_tcont_packet_info((TCONT_COUNTERS_T *)arg);
				break;
            case IO_IOG_GET_ONLINE_DURATION:
                ret = xmcs_get_pon_online_duration((ulong*) arg);
			    break;	
#ifdef TCSUPPORT_WAN_GPON
			case IF_IOG_GEMPORT_INFO : 
                {
                    struct XMCS_GemPortInfo_S *temp = NULL;
					temp = kzalloc(sizeof(struct XMCS_GemPortInfo_S), GFP_ATOMIC);
					if(NULL == temp){
						printk("func:%s line:%d kzalloc fail\n",__func__,__LINE__);
						return -1;
					}
				    ret = xmcs_get_gem_port_info(temp) ;
                    COPY_TO_USER((struct XMCS_GemPortInfo_S *)arg,temp,sizeof(struct XMCS_GemPortInfo_S),ret);
                    kfree(temp);
				}
				break ;
			case IF_IOG_TCONT_INFO : 
                {
                    struct XMCS_TcontInfo_S temp;
                    memset(&temp, 0, sizeof(struct XMCS_TcontInfo_S));
				    ret = xmcs_get_tcont_info(&temp) ;
                    COPY_TO_USER((struct XMCS_TcontInfo_S *)arg,&temp,sizeof(struct XMCS_TcontInfo_S),ret);
                }
				break ;
            case IF_IOG_OMCC_INFO : 
                {
                    struct XMCS_OMCC_Info_S temp = {0};
                    ret = xmcs_get_omcc_info(&temp) ;
                    COPY_TO_USER((struct XMCS_OMCC_Info_S *)arg,&temp,sizeof(struct XMCS_OMCC_Info_S),ret);
                }
            break ;
#endif /* TCSUPPORT_WAN_GPON */
#ifdef TCSUPPORT_WAN_EPON
			case IF_IOG_LLID_INFO : 
                {
                    struct XMCS_EponLlidInfo_S temp;
                    memset(&temp, 0, sizeof(struct XMCS_EponLlidInfo_S));
				    ret = xmcs_get_llid_info(&temp) ;
                    COPY_TO_USER((struct XMCS_EponLlidInfo_S *)arg,&temp,sizeof(struct XMCS_EponLlidInfo_S),ret);
                }
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


