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

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <ecnt_hook/ecnt_hook_fe.h>
#include <ecnt_hook/ecnt_hook_qdma.h>

#include "common/xpon_global.h"
#include "xmcs/xmcs_if.h"	
#include "epon/epon_act.h"
#include "common/phy_if_wrapper.h"
#include "common/xpon_api.h"

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
extern int storm_ctrl_shrehold;
extern int qdma_fwd_timer;
extern int storm_ctrl_shrehold;
extern int qdma_fwd_timer;
extern int auto_mode_flag;
extern void xpon_stop_timer(void);
#ifdef TCSUPPORT_CUC
extern void gpon_start_silence(void);
extern void gpon_stop_silence(void);
#endif

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/

extern int epon_mpcp_register_done(void);
#ifdef TCSUPPORT_CPU_ARMV8
extern int get_ethaddr(unsigned char *ethaddr, int len);
#endif
/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/
int mulitcast_ani = -1;

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
**		xmcs_set_trtcm_congest_mode
**	description:
**		Reserve for configure gpon tcont conguestion mode.
**	parameters:
**		pCongest:
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int xmcs_set_trtcm_congest_mode(struct XMCS_TxQueueCongestion_S *pCongest)
{
	QDMA_Mode_t mode ;
	
	mode = (pCongest->congestMode.trtcm==XPON_ENABLE) ? QDMA_ENABLE : QDMA_DISABLE ;	

	/*todo: not implement yet*/

	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		xmcs_set_congest_threshold
**	description:
**		Reserve for configure gpon tcont conguestion threshold.
**	parameters:
**		pCongest:
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int xmcs_set_congest_threshold(struct XMCS_TxQueueCongestion_S *pCongest)
{
	/*todo: not implement yet*/
	return 0;
}
/*______________________________________________________________________________
**	function name
**		xmcs_get_congest_info
**	description:
**		Reserve for gpon get tcont congest info.
**	parameters:
**		pCongest:
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int xmcs_get_congest_info(struct XMCS_TxQueueCongestion_S *pCongest)
{
	/*todo: not implement yet*/
	/*QDMA_TxQueueCongestCfg_T getCongest ;*/

	return 0;
}
/*______________________________________________________________________________
**	function name
**		xmcs_set_congest_scale
**	description:
**		API for set qos  congestion scale.
**	parameters:
**		pCongest:
**	global:
**		None
**	return:
**		0: scuess
**		-EINVAL: unknow type.
**	call:
**		xpon_set_qdma_qos
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int xmcs_set_congest_scale(struct XMCS_TxQueueCongestion_S *pCongest)
{
	QDMA_TxQueueCongestScale_T setScale ;
	
	if(pCongest->scale.max == XMCS_IF_CONGESTIOM_SCALE_2) {
		setScale.maxScale = QDMA_TXQUEUE_SCALE_2_DSCP ;
	} 
	else if(pCongest->scale.max == XMCS_IF_CONGESTIOM_SCALE_4) {
		setScale.maxScale = QDMA_TXQUEUE_SCALE_4_DSCP ;
	}
	else if(pCongest->scale.max == XMCS_IF_CONGESTIOM_SCALE_8) {
		setScale.maxScale = QDMA_TXQUEUE_SCALE_8_DSCP ;
	}
	else if(pCongest->scale.max == XMCS_IF_CONGESTIOM_SCALE_16) {
		setScale.maxScale = QDMA_TXQUEUE_SCALE_16_DSCP ;
	}
	else {
		return -EINVAL ;
	}
	
	if(pCongest->scale.min == XMCS_IF_CONGESTIOM_SCALE_2) {
		setScale.minScale = QDMA_TXQUEUE_SCALE_2_DSCP ;
	} 
	else if(pCongest->scale.min == XMCS_IF_CONGESTIOM_SCALE_4) {
		setScale.minScale = QDMA_TXQUEUE_SCALE_4_DSCP ;
	}
	else if(pCongest->scale.min == XMCS_IF_CONGESTIOM_SCALE_8) {
		setScale.minScale = QDMA_TXQUEUE_SCALE_8_DSCP ;
	}
	else if(pCongest->scale.min == XMCS_IF_CONGESTIOM_SCALE_16) {
		setScale.minScale = QDMA_TXQUEUE_SCALE_16_DSCP ;
	}
	else {
		return -EINVAL ;
	}

	/*todo: not implement yet*/
	return 0;
}
/*______________________________________________________________________________
**	function name
**		xmcs_set_tx_trtcm_scale
**	description:
**		API for set tx trtcm scale.
**	parameters:
**		pScale:
**	global:
**		None
**	return:
**		0: scuess
**		-EINVAL: unknow type.
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int xmcs_set_tx_trtcm_scale(struct XMCS_TxTrtcmScale_S *pScale)
{
	if(pScale->trtcmScale<0 || pScale->trtcmScale>=XMCS_IF_TRTCM_SCALE_ITEMS) {
		return -EINVAL ;
	}
	
	/*todo: not implement yet*/
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		xmcs_set_qos_weight_config
**	description:
**		API for set qos parameters with weight and  scale.
**	parameters:
**		pWeightCfg:
**	global:
**		None
**	return:
**		0: scuess
**		-EINVAL: unknow type.
**	call:
**		QDMA_API_SET_TX_QOS_WEIGHT
**	revision:
**		v1.0
**____________________________________________________________________________*/
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
        if(pWeightCfg->weightScale == XMCS_IF_WEIGHT_SCALE_1B) {
			qdmaWeightScale = QDMA_TXQOS_WEIGHT_SCALE_1B ;
		} else if(pWeightCfg->weightScale == XMCS_IF_WEIGHT_SCALE_16B) {
			qdmaWeightScale = QDMA_TXQOS_WEIGHT_SCALE_16B ;
		} else {
			return -EINVAL ;
		}
	}
	return QDMA_API_SET_TX_QOS_WEIGHT(ECNT_QDMA_WAN, qdmaWeightType, qdmaWeightScale) ;
}
/*______________________________________________________________________________
**	function name
**		xmcs_get_qos_weight_config
**	description:
**		API for get qos parameters with weight and  scale.
**	parameters:
**		pWeightCfg:
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		QDMA_API_GET_TX_QOS_WEIGHT
**	revision:
**		v1.0
**____________________________________________________________________________*/
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
    pWeightCfg->weightScale = (qdmaWeightScale==QDMA_TXQOS_WEIGHT_SCALE_1B) ? XMCS_IF_WEIGHT_SCALE_1B : XMCS_IF_WEIGHT_SCALE_16B ;
		
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		xmcs_set_channel_scheduler
**	description:
**		API for set qos scheduler mode.
**	parameters:
**		pScheduler:
**	global:
**		None
**	return:
**		0: scuess
**		-EINVAL: unknow type.
**	call:
**		QDMA_API_SET_TX_QOS
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xmcs_set_channel_scheduler(struct XMCS_ChannelQoS_S *pScheduler)
{
	QDMA_TxQosScheduler_T qosScheduler ;
	int i = 0 ;
	
	if(pScheduler->channel>= XPON_CHANNEL_NUMBER) {
		return -EINVAL ;
	}
	qosScheduler.channel = pScheduler->channel ;
	
	if(pScheduler->qosType == XMCS_IF_QOS_TYPE_WRR) {
		qosScheduler.qosType = QDMA_TXQOS_TYPE_WRR ;
	} 
	else if(pScheduler->qosType == XMCS_IF_QOS_TYPE_SP) {
		qosScheduler.qosType = QDMA_TXQOS_TYPE_SP ;
	} 
	else if(pScheduler->qosType == XMCS_IF_QOS_TYPE_SPWRR7) {
		qosScheduler.qosType = QDMA_TXQOS_TYPE_SPWRR7 ;
	} 
	else if(pScheduler->qosType == XMCS_IF_QOS_TYPE_SPWRR6) {
		qosScheduler.qosType = QDMA_TXQOS_TYPE_SPWRR6 ;
	} 
	else if(pScheduler->qosType == XMCS_IF_QOS_TYPE_SPWRR5) {
		qosScheduler.qosType = QDMA_TXQOS_TYPE_SPWRR5 ;
	} 
	else if(pScheduler->qosType == XMCS_IF_QOS_TYPE_SPWRR4) {
		qosScheduler.qosType = QDMA_TXQOS_TYPE_SPWRR4 ;
	}
	else if(pScheduler->qosType == XMCS_IF_QOS_TYPE_SPWRR3) {
		qosScheduler.qosType = QDMA_TXQOS_TYPE_SPWRR3 ;
	}
	else if(pScheduler->qosType == XMCS_IF_QOS_TYPE_SPWRR2) {
		qosScheduler.qosType = QDMA_TXQOS_TYPE_SPWRR2 ;
	}
	else {
		return -EINVAL ;
	}
		
	for(i=0 ; i<CONFIG_QDMA_QUEUE ; i++) {
		qosScheduler.queue[i].weight = pScheduler->queue[i].weight ;
	}
	
	return QDMA_API_SET_TX_QOS(ECNT_QDMA_WAN, &qosScheduler);
}
/*______________________________________________________________________________
**	function name
**		xmcs_get_channel_scheduler
**	description:
**		API for get qos scheduler mode.
**	parameters:
**		pScheduler:
**	global:
**		None
**	return:
**		0: scuess
**		-EINVAL: unknow type.
**	call:
**		QDMA_API_GET_TX_QOS
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int xmcs_get_channel_scheduler(struct XMCS_ChannelQoS_S *pScheduler)
{
	QDMA_TxQosScheduler_T qosScheduler ;
	int i = 0, ret=0 ;
	
	if((pScheduler->channel>=CONFIG_GPON_10G_MAX_TCONT)) {
		return -EINVAL ;
	}
	qosScheduler.channel = pScheduler->channel ;
	
	ret = QDMA_API_GET_TX_QOS(ECNT_QDMA_WAN, &qosScheduler);

	if(ret != 0) {
		return ret ;
	}
	
	if(qosScheduler.qosType == QDMA_TXQOS_TYPE_WRR) {
		pScheduler->qosType = XMCS_IF_QOS_TYPE_WRR ;
	} 
	else if(qosScheduler.qosType == QDMA_TXQOS_TYPE_SP) {
		pScheduler->qosType = XMCS_IF_QOS_TYPE_SP ;
	}
	else if(qosScheduler.qosType == QDMA_TXQOS_TYPE_SPWRR7) {
		pScheduler->qosType = XMCS_IF_QOS_TYPE_SPWRR7 ;
	}
	else if(qosScheduler.qosType == QDMA_TXQOS_TYPE_SPWRR6) {
		pScheduler->qosType = XMCS_IF_QOS_TYPE_SPWRR6 ;
	}
	else if(qosScheduler.qosType == QDMA_TXQOS_TYPE_SPWRR5) {
		pScheduler->qosType = XMCS_IF_QOS_TYPE_SPWRR5 ;
	}
	else if(qosScheduler.qosType == QDMA_TXQOS_TYPE_SPWRR4) {
		pScheduler->qosType = XMCS_IF_QOS_TYPE_SPWRR4 ;
	}
	else if(qosScheduler.qosType == QDMA_TXQOS_TYPE_SPWRR3) {
		pScheduler->qosType = XMCS_IF_QOS_TYPE_SPWRR3 ;
	}
	else if(qosScheduler.qosType == QDMA_TXQOS_TYPE_SPWRR2) {
		pScheduler->qosType = XMCS_IF_QOS_TYPE_SPWRR2 ;
	}
	else{
		return -EINVAL ;
	}
	
	for(i=0 ; i<XPON_QUEUE_NUMBER ; i++) {
		pScheduler->queue[i].weight = qosScheduler.queue[i].weight ;
	}
	
	return ret ;
}
/*______________________________________________________________________________
**	function name
**		xmcs_set_dei_congest_mode
**	description:
**		API for set qos DEI congestion mode.
**	parameters:
**		pCongest:
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		QDMA_API_SET_TXQ_DEI_DROP_MODE
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int xmcs_set_dei_congest_mode(struct XMCS_TxQueueCongestion_S *pCongest)
{
	QDMA_Mode_t mode ;
	
	mode = (pCongest->congestMode.dei==XPON_ENABLE) ? QDMA_ENABLE : QDMA_DISABLE ;	

	QDMA_API_SET_TXQ_DEI_DROP_MODE(ECNT_QDMA_WAN, mode);
	
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		xmcs_set_threshold_congest_mode
**	description:
**		API for set qos threshold congestion mode.
**	parameters:
**		pCongest:
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		xpon_set_qdma_qos
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int xmcs_set_threshold_congest_mode(struct XMCS_TxQueueCongestion_S *pCongest)
{
	QDMA_Mode_t mode ;

	if (pCongest->congestMode.threshold != XPON_OTHER){
		mode = (pCongest->congestMode.threshold==XPON_ENABLE) ? QDMA_ENABLE : QDMA_DISABLE ;	
		/*todo: not implement yet*/
	}
	
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		xmcs_get_tx_trtcm_scale
**	description:
**		Reserve for get trtcm scale.
**	parameters:
**		pScale:
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
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
#if 0
	/*undo*/
	/* qdmaTrtcmScale = get_trtcm_scale_api();*/
	if(qdmaTrtcmScale >= QDMA_TRTCM_SCALE_MAX_ITEMS) {
		return -EFAULT ;
	}
#endif

	pScale->trtcmScale = xmcsTrtcmScale[qdmaTrtcmScale] ;
	
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		xmcs_set_tx_trtcm_params
**	description:
**		
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
**____________________________________________________________________________*/
static int xmcs_set_tx_trtcm_params(struct XMCS_TxQueueTrtcm_S *pTrtcmParams)
{
    GENERAL_TrtcmCbsPbsSet_T qdmaTrtcmParams;
	
	if((pTrtcmParams->tsIdx>=XPON_SHAPING_NUMBER)) {
		return -EINVAL ;
	}
	qdmaTrtcmParams.Index = pTrtcmParams->tsIdx ;
	qdmaTrtcmParams.CbsParamValue = pTrtcmParams->cirValue ;
	qdmaTrtcmParams.CbsBucketSize = pTrtcmParams->cbsUnit ;
	qdmaTrtcmParams.PbsParamValue = pTrtcmParams->pirValue ;
	qdmaTrtcmParams.PbsBucketSize = pTrtcmParams->pbsUnit ;
    qdmaTrtcmParams.trtcmModule = EGRESS_TRTCM ;
	QDMA_API_SET_GENERAL_TRTCM_VALUE(ECNT_QDMA_WAN,&qdmaTrtcmParams);
	/*todo: not implement yet*/
	return 0;
}
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
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
**____________________________________________________________________________*/
static int xmcs_get_tx_trtcm_params(struct XMCS_TxQueueTrtcm_S *pTrtcmParams)
{
    GENERAL_TrtcmCbsPbsSet_T qdmaTrtcmParams;
	int ret = 0 ;
	
	if(pTrtcmParams->tsIdx>=XPON_SHAPING_NUMBER) {
		return -EINVAL ;
	}

    qdmaTrtcmParams.trtcmModule = EGRESS_TRTCM ;
    qdmaTrtcmParams.Index = pTrtcmParams->tsIdx ;

    ret = QDMA_API_GET_GENERAL_TRTCM_VALUE(ECNT_QDMA_WAN,&qdmaTrtcmParams);
	/*todo: not implement yet*/
	if(ret != 0) {
		return ret ;
	}
		
	pTrtcmParams->cirValue = qdmaTrtcmParams.CbsParamValue ;
	pTrtcmParams->cbsUnit = qdmaTrtcmParams.CbsBucketSize ;
	pTrtcmParams->pirValue = qdmaTrtcmParams.PbsParamValue ;
	pTrtcmParams->pbsUnit = qdmaTrtcmParams.PbsBucketSize ;
	
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
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
**____________________________________________________________________________*/
static int xmcs_set_pcp_config(struct XMCS_TxPCPConfig_S *pPcpCfg)
{
	PSE_PcpType_t psePcpType ;
	PSE_PcpMode_t psePcpMode ;
	
	if(pPcpCfg->pcpType == XMCS_IF_PCP_TYPE_CDM_TX) {
		psePcpType = PSE_PCP_TYPE_CDM_TX ;
	} 
	else if(pPcpCfg->pcpType == XMCS_IF_PCP_TYPE_CDM_RX) {
		psePcpType = PSE_PCP_TYPE_CDM_RX ;
	} 
	else if(pPcpCfg->pcpType == XMCS_IF_PCP_TYPE_GDM_RX) {
		psePcpType = PSE_PCP_TYPE_GDM_RX ;
	}
	else {
		return -EINVAL ;
	}
	
	if(pPcpCfg->pcpMode == XMCS_IF_PCP_MODE_DISABLE) {
		psePcpMode = PSE_PCP_MODE_DISABLE ;
	}
	else if(pPcpCfg->pcpMode == XMCS_IF_PCP_MODE_8B0D) {
		psePcpMode = PSE_PCP_MODE_8B0D ;
	}
	else if(pPcpCfg->pcpMode == XMCS_IF_PCP_MODE_7B1D) {
		psePcpMode = PSE_PCP_MODE_7B1D ;
	}
	else if(pPcpCfg->pcpMode == XMCS_IF_PCP_MODE_6B2D) {
		psePcpMode = PSE_PCP_MODE_6B2D ;
	}
	else if(pPcpCfg->pcpMode == XMCS_IF_PCP_MODE_5B3D) {
		psePcpMode = PSE_PCP_MODE_5B3D ;
	}
	else {
		return -EINVAL ;
	} 	

	/*todo: not implement yet*/
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
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
**____________________________________________________________________________*/
static int xmcs_get_pcp_config(struct XMCS_TxPCPConfig_S *pPcpCfg)
{
    printk("xmcs_get_pcp_config has no PPE API NOW\n");
#if 0
    PSE_PcpType_t psePcpType ;
	PSE_PcpMode_t psePcpMode ;
	
	if(pPcpCfg->pcpType == XMCS_IF_PCP_TYPE_CDM_TX) {
		psePcpType = PSE_PCP_TYPE_CDM_TX ;
	}
	else if(pPcpCfg->pcpType == XMCS_IF_PCP_TYPE_CDM_RX) {
		psePcpType = PSE_PCP_TYPE_CDM_RX ;
	}
	else if(pPcpCfg->pcpType == XMCS_IF_PCP_TYPE_GDM_RX) {
		psePcpType = PSE_PCP_TYPE_GDM_RX ;
	}
	else {
		return -EINVAL ;
	}
	
	/*todo: not implement yet*/

	if(psePcpMode == PSE_PCP_MODE_DISABLE) {
		pPcpCfg->pcpMode = XMCS_IF_PCP_MODE_DISABLE ;
	}
	else if(psePcpMode == PSE_PCP_MODE_8B0D) {
		pPcpCfg->pcpMode = XMCS_IF_PCP_MODE_8B0D ;
	}
	else if(psePcpMode == PSE_PCP_MODE_7B1D) {
		pPcpCfg->pcpMode = XMCS_IF_PCP_MODE_7B1D ;
	}
	else if(psePcpMode == PSE_PCP_MODE_6B2D) {
		pPcpCfg->pcpMode = XMCS_IF_PCP_MODE_6B2D ;
	}
	else if(psePcpMode == PSE_PCP_MODE_5B3D) {
		pPcpCfg->pcpMode = XMCS_IF_PCP_MODE_5B3D ;
	}
	else {
		return -EFAULT ;
	} 
#endif	
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
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
**____________________________________________________________________________*/
void prepare_epon(void){
	if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON || gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_TURBO_EPON){
		XPON_PHY_FW_READY_DISABLE();
	    XPON_PHY_SET_MODE(PHY_EPON_CONFIG);
		XPON_PHY_FW_READY_ENABLE();
	}else{
    	XPON_PHY_SET_MODE(PHY_EPON_CONFIG);
	}
	if(XPON_PHY_GET(PON_GET_PHY_READY_STATUS) == PHY_TRUE){
		epon_stop(NULL);
		epon_reset(NULL);
		gpPhyData->phy_link_status = PHY_LINK_STATUS_READY;
		epon_msg_route_dispatch(EPON_MSG_DETECT_READY,NULL);
	}else{
    	epon_reset(NULL);
	}
	WRITE_FE_REG(0xbfb59640,0x0);
}
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
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
**____________________________________________________________________________*/
int xmcs_set_connection_start(XPON_Mode_t mode)
{
    XPON_DPRINT_MSG("mode=%d\n", mode);
    /* notify XPON PHY Driver get ready to handle interrupts*/
	if(auto_mode_flag == AUTOMODE_XEPON1G_TURBOEPON)
		XPON_PHY_LDDLA_AUTO_MODE_ENABLE();

	if(XPON_ENABLE == mode || XPON_OTHER == mode){
		printk("ponTime:Connection Start\n");		
		printk("gpPhyData->working_mode: %d\n", gpPhyData->working_mode);
        switch(gpPhyData->working_mode)
        {
            case PHY_GPON_CONFIG:
            case PHY_XGPON_CONFIG:
			case PHY_XGSPON_CONFIG:
			case PHY_NGPON2_10G_10G_CONFIG:
			case PHY_NGPON2_10G_2G_CONFIG:
			case PHY_NGPON2_2G_2G_CONFIG:
                if(XPON_OTHER == mode)
                    gpon_disable(GPON_MAC_WITH_PHY_RESET);
                else
                    gpon_disable(GPON_MAC_PLAIN_RESET);
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
		gpPonSysData->sysStartup = PON_WAN_START ;
		XPON_PHY_SET(PON_SET_PHY_START) ;
		XPON_START_TIMER(gpPhyData->trans_status_refresh_timer,1000) ;/* 1,000 ms */
	}else if((mode == XPON_DISABLE) || (mode ==XPON_POWER_DOWN) ) {
            XPON_PHY_SET(PON_SET_PHY_STOP) ;
            gpPonSysData->sysStartup = PON_WAN_STOP ;
            gpPhyData->phy_link_status = PHY_LINK_STATUS_LOS;

            switch(gpPhyData->working_mode)
            {
                case PHY_GPON_CONFIG:
        			gpon_disable(GPON_MAC_PLAIN_RESET) ;
                    break;

                case PHY_EPON_CONFIG:
			        epon_stop(NULL) ;
                    break;

                default:
                    break;
            }
            xpon_stop_timer() ;
    }
	return 0 ;
}

int xmcs_set_automode_flag(int flag)
{	
	auto_mode_flag = flag;
	printk("%s.%d flag = %d, auto_mode_flag = %d\n", __FUNCTION__,__LINE__,flag,auto_mode_flag);
	return 0;
}

/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
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
**____________________________________________________________________________*/
int xmcs_get_onu_type(XMCSIF_OnuType_t *type)
{
	if(NULL == type)
		return -EINVAL ;
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
	*type = XMCS_IF_ONU_TYPE_HGU;
	return 0;
#endif/*TCSUPPORT_COMPILE*/
	if(gpPonSysData->sysOnuType == PON_ONU_TYPE_HGU) {
		*type = XMCS_IF_ONU_TYPE_HGU;
	}
	else if(gpPonSysData->sysOnuType == PON_ONU_TYPE_SFU) {
		*type = XMCS_IF_ONU_TYPE_SFU;
	} 
	else {
		*type = XMCS_IF_ONU_TYPE_UNKNOWN;
	}
	
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
	if(NULL == onuMac)
		return -EINVAL ;
	
	getPonMacfromflash(onuMac->mac);

	return 0;
}

/*
______________________________________________________________________________
**	function name
**		xmcs_get_onu_mode
**	description:
**		Reserve for get onu mode api function.
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
int xmcs_get_onu_mode(XMCSIF_WanDetectionMode_t *mode)
{
	if(NULL == mode)
		return -EINVAL ;
	*mode = gpPonSysData->sysPonMode;
	
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

/******************************************************************************
******************************************************************************/
/*______________________________________________________________________________
**  function name
**      getPonModefromflash
**  description:
**      get true mode from flash
**  parameters:
**      null:
**  global:
**      None
**  return:
**      0: scuess
**      -1:fail
**  call:
**      None
**  revision:
**      v1.0
**____________________________________________________________________________*/

int getPonModefromflash(void)
{
    if((gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_XGPON)\
                ||(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_XGSPON)\
                ||(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_GPON)\
                ||(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G)\
                ||(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_2G)\
                ||(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_2G_2G))
    {
        gpPhyData->working_mode = PHY_GPON_CONFIG;
    } else if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_GPON_SYM)
    {
		 gpPhyData->working_mode = PHY_GPON_SYM_CONFIG;
    } else{
        gpPhyData->working_mode = PHY_EPON_CONFIG;
    }
    gpPhyData->is_fix_mode  = TRUE;
    return 0;
}

int getPonMacfromflash(unchar * mac_addr)
{
	unsigned char devMac[EPON_MAC_ADDRESS_LENTH] = {0};
#ifndef TCSUPPORT_CPU_ARMV8
	int i = 0;
#endif
	if(NULL == mac_addr)
		return -EINVAL ;

#ifdef TCSUPPORT_CPU_ARMV8
    if(get_ethaddr(devMac, sizeof(devMac)) != 0){
        printk("func:%s get ethaddr error!!!\n",__func__);
    }
#else
	for (i = 0; i < ETH_ADDR_LEN; i++)
		devMac[i] = READ_FLASH_BYTE(flash_base + EPON_MAC_ADDRESS_OFFSET + i);
#endif
	if (
		devMac[0] == 0 && devMac[1] == 0 && devMac[2] == 0 &&
		devMac[3] == 0 && devMac[4] == 0 && devMac[5] == 0
	)
		printk(KERN_INFO "\nThe MAC address in flash is null! Use default!\n");		
	else	
		memcpy(mac_addr, devMac, sizeof(devMac));

	return 0;
}

int xmcs_set_link_detection(XMCSIF_WanDetectionMode_t detection)
{
	if(gpPonSysData->sysStartup == PON_WAN_START) {
		return 0;
	}
	
    XPON_DPRINT_MSG("detection: %d\n", detection);
    switch(detection)
    {
        case XMCS_IF_WAN_DETECT_MODE_GPON:
        case XMCS_IF_WAN_DETECT_MODE_XGPON:
        case XMCS_IF_WAN_DETECT_MODE_XGSPON:
		case XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G:
		case XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_2G:
		case XMCS_IF_WAN_DETECT_MODE_NGPON2_2G_2G:
            gpPhyData->is_fix_mode  = TRUE;
            gpPhyData->working_mode = PHY_GPON_CONFIG;
            break;
						
        case XMCS_IF_WAN_DETECT_MODE_EPON:
            gpPhyData->is_fix_mode  = TRUE;
            gpPhyData->working_mode = PHY_EPON_CONFIG;

            break;
            
        case XMCS_IF_WAN_DETECT_MODE_10G_1G_EPON:
            gpPhyData->is_fix_mode  = TRUE;
            gpPhyData->working_mode = PHY_EPON_CONFIG;

            break;
            
        case XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON:
            gpPhyData->is_fix_mode  = TRUE;
            gpPhyData->working_mode = PHY_EPON_CONFIG;

            break;
			
        case XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON:
            gpPhyData->is_fix_mode  = TRUE;
            gpPhyData->working_mode = PHY_EPON_CONFIG;
            break;

		case XMCS_IF_WAN_DETECT_MODE_TURBO_EPON:
            gpPhyData->is_fix_mode  = TRUE;
            gpPhyData->working_mode = PHY_EPON_CONFIG;
            break;

        default:
            getPonModefromflash();
	}

    return 0;
}

/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
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
**____________________________________________________________________________*/
int xmcs_get_wan_link_status(struct XMCS_WanLinkConfig_S *pSysLinkStatus)
{
	pSysLinkStatus->linkStart = (gpPonSysData->sysStartup==PON_WAN_START) ? XPON_ENABLE : XPON_DISABLE ;
	
    pSysLinkStatus->detectMode = gpPonSysData->sysPonMode ;

	if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) {
		pSysLinkStatus->linkStatus = XMCS_IF_WAN_LINK_GPON ;
	} else if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON) {
		pSysLinkStatus->linkStatus = XMCS_IF_WAN_LINK_EPON ;
	} else {
		pSysLinkStatus->linkStatus = XMCS_IF_WAN_LINK_OFF ;
	} 

	return XMCS_IF_WAN_LINK_OFF ;
}

/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xmcs_set_wan_cnt_stats(uint mask)
{
	if(mask&0x01)
		FE_API_SET_CLEAR_MIB(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX);
	else if(mask&0x02)
		FE_API_SET_CLEAR_MIB(FE_GDM_SEL_GDMA2, FE_GDM_SEL_RX);

	return 0;
}

/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xmcs_get_wan_cnt_stats(WAN_STATISTIC_t *pWanCntStats)
{
	REG_RX_HDR_HEC_CNT		   rx_hdr_hec_cnt;	
	PHY_FecCount_T phyFecCount ;
	struct XMCS_WanCntStats_S wan_cnt_temp = {0};
	FE_TxCnt_t txCnt;
	FE_RxCnt_t rxCnt;
	
	if (pWanCntStats == NULL)
		return -EINVAL;

    
    
	if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) {
		rx_hdr_hec_cnt.Raw =		IO_GREG(RX_HDR_HEC_CNT) ;
		wan_cnt_temp.rxHecErrorCnt = rx_hdr_hec_cnt.Bits.rx_hdr_hec_1err_cnt + \
			rx_hdr_hec_cnt.Bits.rx_hdr_hec_2err_cnt + rx_hdr_hec_cnt.Bits.rx_hdr_hec_3err_cnt ;
	}else{
		wan_cnt_temp.rxHecErrorCnt = 0;
	}

    if(rdk_gtc_dbg.proc_flag) {
        wan_cnt_temp.rxHecErrorCnt = rdk_gtc_dbg.hecCounter.XGEMHECErrCount ;
        rdk_gtc_dbg.proc_flag = 0;
    }

    XPON_PHY_GET_API(PON_GET_PHY_RX_FEC_COUNTER, &phyFecCount);

	wan_cnt_temp.rxFecErrorCnt = gphyFecCount.uncorrect_codewords;
	wan_cnt_temp.rxFecCerrorCnt= gphyFecCount.correct_codewords;

	if( PHY_LINK_STATUS_READY == gpPhyData->phy_link_status )
	{
		wan_cnt_temp.rxFecErrorCnt += phyFecCount.uncorrect_codewords;
		wan_cnt_temp.rxFecCerrorCnt+= phyFecCount.correct_codewords;
	}
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
	if(0 != copy_to_user(pWanCntStats, &wan_cnt_temp, sizeof(wan_cnt_temp)))
	{
		printk("func:[%s] line:[%d] calling copy_to_user fail \n",__FUNCTION__,__LINE__);
		return -1;
	}

	return 0;
}

/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xpon_reset_qdma_tx_buf(void ){
	QDMA_TxBufCtrl_T txBufCtrl;

	txBufCtrl.mode = QDMA_ENABLE;
	
	if(isEN7581){
		 if(gpWanPriv->activeChannelNum <=8){
	    	txBufCtrl.chnThreshold = 0x40;
	    	txBufCtrl.totalThreshold = (0x20+0x10-1)*(gpWanPriv->activeChannelNum)*2;
		}else{
			txBufCtrl.chnThreshold = 0x10;
			txBufCtrl.totalThreshold = ((0x08+0x10-1)*(gpWanPriv->activeChannelNum-1) + 1)*2;
		}
	} else if(isAN7583) {
		txBufCtrl.chnThreshold = 0x30;
		txBufCtrl.totalThreshold = (0x30)*(gpWanPriv->activeChannelNum);
	} else {	
		txBufCtrl.chnThreshold = (unchar)min(0x20, 0x80/(unchar)gpWanPriv->activeChannelNum);
		txBufCtrl.totalThreshold = 0x80;
	}
	
	QDMA_API_SET_TXBUF_THRESHOLD(ECNT_QDMA_WAN, &txBufCtrl);
	
	return 0;
}

/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
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

#ifdef TCSUPPORT_WAN_GPON
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int xmcs_assign_gem_port(struct XMCS_GemPortAssign_S *pGemAssign)
{
	int i = 0, ret = 0;
	
	if((pGemAssign==NULL) || (pGemAssign->entryNum>=CONFIG_GPON_10G_MAX_GEMPORT)) {
		return -EINVAL ;
	}
	
	for(i=0 ; i<pGemAssign->entryNum ; i++) {
		if(pGemAssign->gemPort[i].ani >= GPON_MAX_ANI_INTERFACE) {
			return -EINVAL ;
		}

		if(!gpWanPriv->pPonNetDev[PWAN_IF_DATA]) {
			return -EINVAL ;
		}
	
		if(pGemAssign->gemPort[i].id!=GPON_OMCC_ID) {  
			ret = gwan_config_gemport(pGemAssign->gemPort[i].id, ENUM_CFG_NETIDX, pGemAssign->gemPort[i].ani) ;
			if(ret != 0) {
				return ret ;
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
}
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xmcs_unassign_gem_port(struct XMCS_GemPortAssign_S *pGemAssign)
{
	int i =0, ret = 0 ;
	
	if((pGemAssign==NULL) || (pGemAssign->entryNum>=CONFIG_GPON_10G_MAX_GEMPORT)) {
		return -EINVAL ;
	}

	for(i=0 ; i<pGemAssign->entryNum ; i++) {
		if(pGemAssign->gemPort[i].id!=GPON_OMCC_ID) {
			ret = gwan_config_gemport(pGemAssign->gemPort[i].id, ENUM_CFG_NETIDX, GPON_MAX_ANI_INTERFACE) ;
			if(ret != 0) {
				return ret ;
			}
		}
	}	
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xmcs_create_gem_port(struct XMCS_GemPortCreate_S *pGemCreate)
{
	unchar gemIdx = 0, channel=CONFIG_GPON_10G_MAX_TCONT ;
	int i = 0 ;
	int ret = 0;
	
	if(pGemCreate->gemPortId==GPON_OMCC_ID) {
		return -EINVAL ;
	}
	if(pGemCreate->gemType == GPON_UNICAST_GEM) {		
		if(pGemCreate->allocId>=GPON_10G_MAX_ALLOC_ID) {
			return -EINVAL ;
		}
		
		for(i=0 ; i<CONFIG_GPON_10G_MAX_TCONT ; i++) {
			if(gpWanPriv->gpon.allocId[i] == pGemCreate->allocId) {
				channel = i ;
				break ;
			}
		}

		if(i == CONFIG_GPON_10G_MAX_TCONT){
			channel = GPON_UNKNOWN_CHANNEL;
		}
		
	} else if(pGemCreate->gemType == GPON_MULTICAST_GEM) {
		channel = GPON_MULTICAST_CHANNEL ;
	} else {
		return -EINVAL ;
	}

    gemIdx = (gpWanPriv->gpon.gemIdToIndex[pGemCreate->gemPortId] & GPON_GEM_IDX_MASK);
    

    if(gpWanPriv->gpon.gemPort[gemIdx].info.valid && gpWanPriv->gpon.gemPort[gemIdx].info.portId==pGemCreate->gemPortId)
    {
        ret = gwan_config_gemport(pGemCreate->gemPortId, ENUM_CFG_CHANNEL, channel);
    }
    else{
        ret = gwan_create_new_gemport(pGemCreate->gemPortId, channel, pGemCreate->gemType,pGemCreate->allocId);
    }

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT) || defined(TCSUPPORT_FWC_ENV)
    if(0 == ret && gpWanPriv->gpon.gemPort[gemIdx].info.ani != 1)
    {
        printk("Fh creat gemport %d, add assign \n",pGemCreate->gemPortId);
        assignGemportId(pGemCreate->gemPortId);
    }
#endif/*TCSUPPORT_COMPILE*/

	return ret;
}
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xmcs_remove_gem_port(ushort gemPortId)
{
	if(gemPortId>=GPON_MAX_GEM_ID || gemPortId == GPON_OMCC_ID) {
		return -EINVAL ;
	}	

	if(GPON_OMCC_ID != gemPortId){
		gpon_recover_delete_gemport(gemPortId);
	}

	return gwan_remove_gemport(gemPortId) ;
}

/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xmcs_remove_all_gem_port(void)
{
	
	return gwan_remove_all_gemport() ;
}
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int xmcs_set_gem_port_loopback(struct XMCS_GemPortLoopback_S *pGemLb)
{
	if(pGemLb->gemPortId==GPON_OMCC_ID) {
		return -EINVAL ;
	}
	
	if(pGemLb->loopback == XPON_DISABLE) {
		return gwan_config_gemport(pGemLb->gemPortId, ENUM_CFG_LOOPBACK, 0) ; 
	} else {
		return gwan_config_gemport(pGemLb->gemPortId, ENUM_CFG_LOOPBACK, 1) ;  
	}
	
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xmcs_get_gem_port_info(struct XMCS_GemPortInfo_S *pGemInfo)
{
	int i = 0 ;
	unchar channel = 0 ;
	pGemInfo->entryNum = 0 ;
	for(i=0 ; i<CONFIG_GPON_10G_MAX_GEMPORT ; i++) {
		if(gpWanPriv->gpon.gemPort[i].info.valid) {
			channel = gpWanPriv->gpon.gemPort[i].info.channel ;
			
			if(channel >= GPON_MULTICAST_CHANNEL) {
				pGemInfo->info[pGemInfo->entryNum].gemType = GPON_MULTICAST_GEM ;
				pGemInfo->info[pGemInfo->entryNum].allocId = GPON_10G_UNASSIGN_ALLOC_ID;
			} else {
				pGemInfo->info[pGemInfo->entryNum].gemType = GPON_UNICAST_GEM ;
				pGemInfo->info[pGemInfo->entryNum].allocId = gpWanPriv->gpon.allocId[channel] ;
			}
			pGemInfo->info[pGemInfo->entryNum].gemPortId = gpWanPriv->gpon.gemPort[i].info.portId ;
			pGemInfo->info[pGemInfo->entryNum].aniIdx = gpWanPriv->gpon.gemPort[i].info.ani ;
			pGemInfo->info[pGemInfo->entryNum].lbMode = (gpWanPriv->gpon.gemPort[i].info.rxLb) ? XPON_ENABLE : XPON_DISABLE ;
			pGemInfo->info[pGemInfo->entryNum].enMode = (gpWanPriv->gpon.gemPort[i].info.rxEncrypt) ? XPON_ENABLE : XPON_DISABLE ;
            pGemInfo->info[pGemInfo->entryNum].txEncrypt = (gpWanPriv->gpon.gemPort[i].info.txEncrypt) ? XPON_ENABLE : XPON_DISABLE ;

			pGemInfo->entryNum++ ;
		}
	}
	
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xmcs_get_tcont_info(struct XMCS_TcontInfo_S *pTcontInfo)
{
	int i = 0 ;
	
	pTcontInfo->entryNum = 0 ;
	for(i=0 ; i<CONFIG_GPON_10G_MAX_TCONT ; i++) {
		if(gpWanPriv->gpon.allocId[i] != GPON_10G_UNASSIGN_ALLOC_ID) {
			pTcontInfo->info[pTcontInfo->entryNum].allocId = gpWanPriv->gpon.allocId[i] ;
			pTcontInfo->info[pTcontInfo->entryNum].channel = i ;
			pTcontInfo->entryNum++ ;
		}
	}
	
	return 0 ;
}

/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
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
			xpon_reset_qdma_tx_buf();
		}
	}

	return 0 ;
}

int xmcs_get_omcc_info(struct XMCS_OMCC_Info_S *ptOmccInfo)
{
    struct XMCS_OMCC_Info_S tmpOmccInfo = {0};

    if(NULL == ptOmccInfo)
    {
        return -EINVAL ;
    }
    
    tmpOmccInfo.allocId   = GPON_ONU_ID;
    tmpOmccInfo.gemportid = GPON_OMCC_ID;
	memcpy(ptOmccInfo, &tmpOmccInfo, sizeof(struct XMCS_OMCC_Info_S));

    return 0 ;
}
/***************************************************************
***************************************************************/
int xmcs_set_gem_encrypt(struct XMCS_GemPortCreate_S *pGemCreate)
{
	unchar gemIdx = 0;
	int ret = 0;

	if(pGemCreate == NULL) {
		return -EINVAL ;
	}
	gemIdx = (gpWanPriv->gpon.gemIdToIndex[pGemCreate->gemPortId] & GPON_GEM_IDX_MASK);
	if(gpWanPriv->gpon.gemPort[gemIdx].info.valid && gpWanPriv->gpon.gemPort[gemIdx].info.portId==pGemCreate->gemPortId)
    {
		ret = gwan_config_gemport(pGemCreate->gemPortId, ENUM_CFG_ENCRYPTION, pGemCreate->gemEncrypt);
		if(0 != ret){
			return ret;
		}
    }	
	return ret;
}

#endif /* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xmcs_create_llid(struct XMCS_EponLlidCreate_S *pLlidCreate)
{
	unchar idx = pLlidCreate->idx ;
	
	if(idx >= EPON_LLID_MAX_NUM) {
        printk("ERROR:%s llid index exceed max num \n",__FUNCTION__);
		return -EINVAL ;
	}

	if(gpWanPriv->epon.llid[idx].info.valid) {
        printk("ERROR:%s llid has exist \n",__FUNCTION__);
		return -EEXIST ;
	} else {
		gpWanPriv->epon.llid[idx].info.llid = pLlidCreate->llid ;
		gpWanPriv->epon.llid[idx].info.channel = idx ;
		gpWanPriv->epon.llid[idx].info.valid = 1 ;
		
		return 0 ;
	}
}

/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xmcs_remove_llid(unchar idx)
{
	if(idx >= EPON_LLID_MAX_NUM) {
		return -EINVAL ;
	}

	if(gpWanPriv->epon.llid[idx].info.valid) {
		memset(&gpWanPriv->epon.llid[idx].info, 0, sizeof(EWAN_LlidInfo_T)) ;
		memset(&gpWanPriv->epon.llid[idx].stats, 0, sizeof(struct net_device_stats)) ;
		gpWanPriv->epon.llid[idx].info.rxDrop = 1 ;
		gpWanPriv->epon.llid[idx].info.txDrop = 1 ;
		//pwan_delete_net_interface(idx) ;  
		
		return 0 ;
	} else {
		return -EINVAL ;
	}
}

/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int xmcs_set_epon_rx_config(struct XMCS_EponRxConfig_S *pRxCfg)
{
	unchar idx = pRxCfg->idx ;
	
	if(idx >= EPON_LLID_MAX_NUM) {
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
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int xmcs_set_epon_tx_config(struct XMCS_EponTxConfig_S *pTxCfg)
{
	unchar idx = pTxCfg->idx ;
	
	if(idx >= EPON_LLID_MAX_NUM) {
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
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
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
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xmcs_get_llid_info(struct XMCS_EponLlidInfo_S *pLlidInfo)
{
	int i ;
	
	pLlidInfo->entryNum = 0 ;
	for(i=0 ; i<EPON_LLID_MAX_NUM ; i++) {
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

/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
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
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
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
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xmcs_get_storm_ctrl_config(struct XMCS_StormCtrlConfig_S *pStormCfg)
{
	printk("xmcs_get_storm_ctrl_config: ----- storm_ctrl_shrehold = %d\r\n", storm_ctrl_shrehold);
	printk("xmcs_get_storm_ctrl_config: ----- qdma_fwd_timer = %d\r\n", qdma_fwd_timer);

	pStormCfg->threld = storm_ctrl_shrehold;
	pStormCfg->timer = qdma_fwd_timer;

	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xmcs_get_dbg_level(ushort *level)
{
	*level = gpPonSysData->debugLevel;

	return 0 ;
}

/*______________________________________________________________________________
**  function name
**      xmcs_get_pon_online_duration
**  description:
**      get pon online time
**  parameters:
**      mask: arg 
**  global:
**      None
**  return:
**      0: scuess
**  call:
**      gwan_get_traffic_status
**      epon_mpcp_register_done
**  revision:
**      v1.0
**____________________________________________________________________________*/

static int xmcs_get_pon_online_duration(ulong *arg)
{
	unsigned long duration=0;

	if(arg == NULL){
		return -EINVAL ;
	}
	if( (GPON_TRAFFIC_UP == gwan_get_traffic_status()) \
        ||(MPCP_STATE_REGISTERED == epon_mpcp_register_done()))
	{
        duration = (jiffies-gpPonSysData->onlineStartTime)/HZ;
		memcpy(arg,&duration,sizeof(unsigned long));
		
        return 0;   
	}
    else
	{
		*arg = duration;
		return 0;
	}
}
/*______________________________________________________________________________
**  function name
**      xmcs_set_gpon_silence
**  description:
**      enable/disable the gpon silence
**  parameters:
**      flag 
**  global:
**      None
**  return:
**      0: scuess
**  call:
**      gpon_start_silence
**      gpon_stop_silence
**  revision:
**      v1.0
**____________________________________________________________________________*/
int xmcs_set_gpon_silence(uint flag) 
{
#ifdef TCSUPPORT_CUC
	if(flag == 1){
		gpon_start_silence() ;
	}else{
		gpon_stop_silence() ;
	}
#endif
	return 0;
}

/*______________________________________________________________________________
**	function name
**		xmcs_null
**	description:
**		Reserve for gpon tcont create api function.
**	parameters:
**		mask: rxCntClrMask = 0x02 & txCntClrMask = 0x01  
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
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
					struct XMCS_QoSWeightConfig_S temp;
					memset(&temp, 0, sizeof(struct XMCS_QoSWeightConfig_S));
					COPY_FROM_USER(&temp, (struct XMCS_QoSWeightConfig_S *)arg, sizeof(struct XMCS_QoSWeightConfig_S), ret);
					ret = xmcs_set_qos_weight_config(&temp);
				}
				break ;	
			case IF_IOS_QOS_SCHEDULER :
				{
					struct XMCS_ChannelQoS_S temp ;
					memset(&temp, 0, sizeof(struct XMCS_ChannelQoS_S));
					COPY_FROM_USER(&temp, (struct XMCS_ChannelQoS_S *)arg, sizeof(struct XMCS_ChannelQoS_S), ret);
					ret = xmcs_set_channel_scheduler(&temp);
				}
				break ;	
			case IF_IOS_TRTCM_CONGEST_MODE :
				{
					struct XMCS_TxQueueCongestion_S temp;
					memset(&temp, 0, sizeof(struct XMCS_TxQueueCongestion_S));
					COPY_FROM_USER(&temp, (struct XMCS_TxQueueCongestion_S *)arg, sizeof(struct XMCS_TxQueueCongestion_S), ret);
					ret = xmcs_set_trtcm_congest_mode(&temp);
				}
				break ;	
			case IF_IOS_DEI_CONGEST_MODE :
				{
					struct XMCS_TxQueueCongestion_S temp;
					memset(&temp, 0, sizeof(struct XMCS_TxQueueCongestion_S));
					COPY_FROM_USER(&temp, (struct XMCS_TxQueueCongestion_S *)arg, sizeof(struct XMCS_TxQueueCongestion_S), ret);
					ret = xmcs_set_dei_congest_mode(&temp);
				}
				break ;	
			case IF_IOS_THRESHOLD_CONGEST_MODE :
				{
					struct XMCS_TxQueueCongestion_S temp;
					memset(&temp, 0, sizeof(struct XMCS_TxQueueCongestion_S));
					COPY_FROM_USER(&temp, (struct XMCS_TxQueueCongestion_S *)arg, sizeof(struct XMCS_TxQueueCongestion_S), ret);
					ret = xmcs_set_threshold_congest_mode(&temp);
				}
				break ;	
			case IF_IOS_CONGEST_SCALE :
				{
					struct XMCS_TxQueueCongestion_S temp;
					memset(&temp, 0, sizeof(struct XMCS_TxQueueCongestion_S));
					COPY_FROM_USER(&temp, (struct XMCS_TxQueueCongestion_S *)arg, sizeof(struct XMCS_TxQueueCongestion_S), ret);
					ret = xmcs_set_congest_scale(&temp);
				}
				break ;	
			case IF_IOS_CONGEST_THRESHOLD :
				{
					struct XMCS_TxQueueCongestion_S temp;
					memset(&temp, 0, sizeof(struct XMCS_TxQueueCongestion_S));
					COPY_FROM_USER(&temp, (struct XMCS_TxQueueCongestion_S *)arg, sizeof(struct XMCS_TxQueueCongestion_S), ret);
					ret = xmcs_set_congest_threshold(&temp);
				}
				break ;	
			case IO_IOS_TX_TRTCM_SCALE :
				{
					struct XMCS_TxTrtcmScale_S temp;
					memset(&temp, 0, sizeof(struct XMCS_TxTrtcmScale_S));
					COPY_FROM_USER(&temp, (struct XMCS_TxTrtcmScale_S *)arg, sizeof(struct XMCS_TxTrtcmScale_S), ret);
					ret = xmcs_set_tx_trtcm_scale(&temp);
				}
				break ;	
			case IO_IOS_TX_TRTCM_PARAMS :
				{
					struct XMCS_TxQueueTrtcm_S temp;
					memset(&temp, 0, sizeof(struct XMCS_TxQueueTrtcm_S));
					COPY_FROM_USER(&temp, (struct XMCS_TxQueueTrtcm_S *)arg, sizeof(struct XMCS_TxQueueTrtcm_S), ret);
					ret = xmcs_set_tx_trtcm_params(&temp);
				}
				break ;	
			case IO_IOS_PCP_CONFIG :
				{
					struct XMCS_TxPCPConfig_S temp;
					memset(&temp, 0, sizeof(struct XMCS_TxPCPConfig_S));
					COPY_FROM_USER(&temp, (struct XMCS_TxPCPConfig_S *)arg, sizeof(struct XMCS_TxPCPConfig_S), ret);
					ret = xmcs_set_pcp_config(&temp);
				}
				break ;	
			case IO_IOS_WAN_LINK_START :
				ret = xmcs_set_connection_start((XPON_Mode_t)arg) ;
				break ;					
			case IO_IOS_WAN_DETECTION_MODE :
				ret = xmcs_set_link_detection((XMCSIF_WanDetectionMode_t)arg) ;
				break ;
			case IO_IOS_DETECTION_AUTOMODE :
				ret = xmcs_set_automode_flag((AutoMode_Combo_Type_t )arg);
				break ;
			case IO_IOS_WAN_CNT_STATS:
				ret = xmcs_set_wan_cnt_stats(arg);
				break;
			case IO_IOS_STORM_CTL_CONFIG:
				{
					struct XMCS_StormCtrlConfig_S temp;
					memset(&temp, 0, sizeof(struct XMCS_StormCtrlConfig_S));
					COPY_FROM_USER(&temp, (struct XMCS_StormCtrlConfig_S *)arg, sizeof(struct XMCS_StormCtrlConfig_S), ret);
					ret = xmcs_set_storm_ctrl_config(&temp);
				}
				break;	
			case IO_IOS_DBG_LEVEL : 
				{
					struct XMCS_DebugLevel_S temp;
					memset(&temp, 0, sizeof(struct XMCS_DebugLevel_S));
					COPY_FROM_USER(&temp, (struct XMCS_DebugLevel_S *)arg, sizeof(struct XMCS_DebugLevel_S), ret);
					ret = xmcs_set_dbg_level(&temp);
				}
				break ;
#ifdef TCSUPPORT_WAN_GPON
			case IF_IOS_GEMPORT_ASSIGN : 
				{
					struct XMCS_GemPortAssign_S temp;
					memset(&temp, 0, sizeof(struct XMCS_GemPortAssign_S));
					COPY_FROM_USER(&temp, (struct XMCS_GemPortAssign_S *)arg, sizeof(struct XMCS_GemPortAssign_S), ret);
					ret = xmcs_assign_gem_port(&temp);
				}
				break ;
			case IF_IOS_GEMPORT_UNASSIGN : 
				{
					struct XMCS_GemPortAssign_S temp;
					memset(&temp, 0, sizeof(struct XMCS_GemPortAssign_S));
					COPY_FROM_USER(&temp, (struct XMCS_GemPortAssign_S *)arg, sizeof(struct XMCS_GemPortAssign_S), ret);
					ret = xmcs_unassign_gem_port(&temp);
				}
				break ;
			case IF_IOS_GEMPORT_CREATE : 
				{
					struct XMCS_GemPortCreate_S temp;
					memset(&temp, 0, sizeof(struct XMCS_GemPortCreate_S));
					COPY_FROM_USER(&temp, (struct XMCS_GemPortCreate_S *)arg, sizeof(struct XMCS_GemPortCreate_S), ret);
					ret = xmcs_create_gem_port(&temp);
				}
				break ;
			case IF_IOS_GEMPORT_REMOVE : 
				{
					ushort temp;
					memset(&temp, 0, sizeof(ushort));
					COPY_FROM_USER(&temp, (ushort *)arg, sizeof(ushort), ret);
					ret = xmcs_remove_gem_port(temp);
				}
				break ;
			case IF_IOS_GEMPORT_REMOVE_ALL : 
				ret = xmcs_remove_all_gem_port() ;
				break ;
			case IF_IOS_GEMPORT_LOOPBACK :
				{
					struct XMCS_GemPortLoopback_S temp;
					memset(&temp, 0, sizeof(struct XMCS_GemPortLoopback_S));
					COPY_FROM_USER(&temp, (struct XMCS_GemPortLoopback_S *)arg, sizeof(struct XMCS_GemPortLoopback_S), ret);
					ret = xmcs_set_gem_port_loopback(&temp);
				}
				break ;
			case IF_IOS_TCONT_INFO_REMOVE : 
				{	
					ushort temp;
					memset(&temp, 0, sizeof(ushort));
					COPY_FROM_USER(&temp, (ushort *)arg, sizeof(ushort), ret);
					ret = xmcs_remove_tcont_info(temp) ;
				}
				break ; 
			case IF_IOS_TCONT_INFO_CREATE : 
				{
					struct XMCS_TcontCfg_S temp;
					memset(&temp, 0, sizeof(struct XMCS_TcontCfg_S));
					COPY_FROM_USER(&temp, (struct XMCS_TcontCfg_S *)arg, sizeof(struct XMCS_TcontCfg_S), ret);
					ret = xmcs_create_tcont_info(&temp);
				}
				break ;	
			case IF_IOS_GPON_SILENCE: 
				ret = xmcs_set_gpon_silence((uint)arg);				
				break;
			case IF_IOS_XGEM_ENCRYPT_CFG:
				{
					struct XMCS_GemPortCreate_S temp;
					memset(&temp, 0, sizeof(struct XMCS_GemPortCreate_S));
					COPY_FROM_USER(&temp, (struct XMCS_GemPortCreate_S *)arg, sizeof(struct XMCS_GemPortCreate_S), ret);
					ret = xmcs_set_gem_encrypt(&temp);
				}
				break;
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
			case IO_IOS_QOS_CREAT_POLICER:
				ret = 0;
			break;	
			case IO_IOS_QOS_DELETE_POLICER:
				ret = 0;
			break;	
			case IO_IOS_QOS_OVERALL_RATELIMIT_CONFIG:
				ret = 0;
			break;
#endif	
#if 0 //def TCSUPPORT_WAN_GPON_10G		
			case IO_IOS_PHY_MODE :
				ret = xmcs_set_phy_mode((XGMCSIF_PhyMode_t)arg);
				break ;
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
                    struct XMCS_QoSWeightConfig_S temp;
					memset(&temp, 0, sizeof(struct XMCS_QoSWeightConfig_S));
				    ret = xmcs_get_qos_weight_config(&temp) ;
                    COPY_TO_USER((struct XMCS_QoSWeightConfig_S *)arg,&temp,sizeof(struct XMCS_QoSWeightConfig_S),ret);
				}
				break ;	
			case IF_IOG_QOS_SCHEDULER :
				{
                    struct XMCS_ChannelQoS_S temp;
					memset(&temp, 0, sizeof(struct XMCS_ChannelQoS_S));
					COPY_FROM_USER(&temp,(struct XMCS_ChannelQoS_S *)arg,sizeof(struct XMCS_ChannelQoS_S),ret);
				    ret = xmcs_get_channel_scheduler(&temp) ;
                    COPY_TO_USER((struct XMCS_ChannelQoS_S *)arg,&temp,sizeof(struct XMCS_ChannelQoS_S),ret);
				}
				break ;	
			case IO_IOG_CONGEST_INFO :
				{
                    struct XMCS_TxQueueCongestion_S temp;
					memset(&temp, 0, sizeof(struct XMCS_TxQueueCongestion_S));
				    ret = xmcs_get_congest_info(&temp) ;
                    COPY_TO_USER((struct XMCS_TxQueueCongestion_S *)arg,&temp,sizeof(struct XMCS_TxQueueCongestion_S),ret);
				}
				break ;	
			case IO_IOG_TX_TRTCM_SCALE :
				{
                    struct XMCS_TxTrtcmScale_S temp;
					memset(&temp, 0, sizeof(struct XMCS_TxTrtcmScale_S));
				    ret = xmcs_get_tx_trtcm_scale(&temp) ;
                    COPY_TO_USER((struct XMCS_TxTrtcmScale_S *)arg,&temp,sizeof(struct XMCS_TxTrtcmScale_S),ret);
				}
				break ;	
			case IO_IOG_TX_TRTCM_PARAMS :
				{
                    struct XMCS_TxQueueTrtcm_S temp;
					memset(&temp, 0, sizeof(struct XMCS_TxQueueTrtcm_S));
					COPY_FROM_USER(&temp,(struct XMCS_TxQueueTrtcm_S *)arg,sizeof(struct XMCS_TxQueueTrtcm_S),ret);
				    ret = xmcs_get_tx_trtcm_params(&temp) ;
                    COPY_TO_USER((struct XMCS_TxQueueTrtcm_S *)arg,&temp,sizeof(struct XMCS_TxQueueTrtcm_S),ret);
				}
				break ;	
			case IO_IOG_PCP_CONFIG :
				{
                    struct XMCS_TxPCPConfig_S temp;
					memset(&temp, 0, sizeof(struct XMCS_TxPCPConfig_S));
					COPY_FROM_USER(&temp,(struct XMCS_TxPCPConfig_S *)arg,sizeof(struct XMCS_TxPCPConfig_S),ret);
				    ret = xmcs_get_pcp_config(&temp) ;
                    COPY_TO_USER((struct XMCS_TxPCPConfig_S *)arg,&temp,sizeof(struct XMCS_TxPCPConfig_S),ret);
				}
				break ;	
			case IO_IOG_WAN_LINK_CONFIG :
				{
                    struct XMCS_WanLinkConfig_S temp;
					memset(&temp, 0, sizeof(struct XMCS_WanLinkConfig_S));
					COPY_FROM_USER(&temp,(struct XMCS_WanLinkConfig_S *)arg,sizeof(struct XMCS_WanLinkConfig_S),ret);
					ret = xmcs_get_wan_link_status(&temp);
					COPY_TO_USER((struct XMCS_WanLinkConfig_S *)arg, &temp, sizeof(struct XMCS_WanLinkConfig_S), ret);	
                }
				break ;
			case IO_IOG_ONU_TYPE :
				{
					XMCSIF_OnuType_t temp = XMCS_IF_ONU_TYPE_UNKNOWN;
					ret = xmcs_get_onu_type(&temp);
					COPY_TO_USER((XMCSIF_OnuType_t *)arg, &temp, sizeof(XMCSIF_OnuType_t), ret);
				}
				break ;
			case IO_IOG_BBF_247_FLAG :
				{
					ret = 0;
					COPY_TO_USER((uint8_t *)arg,&gpPonSysData->sysBBF247, sizeof(uint8_t), ret);
				}
				break ;
			case IO_IOG_ONU_MODE :
				{
                   XMCSIF_WanDetectionMode_t temp;
					memset(&temp, 0, sizeof(XMCSIF_WanDetectionMode_t));
				    ret = xmcs_get_onu_mode(&temp) ;
                    COPY_TO_USER((XMCSIF_WanDetectionMode_t *)arg,&temp,sizeof(XMCSIF_WanDetectionMode_t),ret);
				}
				break ;
			case IO_IOG_ONU_MODE_FROM_FLASH	:
				ret = xmcs_get_onu_mode_from_flash((unsigned char *) arg);
				break;
			case IO_IOS_CLR_ROGUE_STATUS :
				ret = xmcs_clear_rogue_status() ;
				break ;
			case IO_IOG_GET_ROGUE_STATUS :
				{
                   PON_RogueStatus_t temp;
					memset(&temp, 0, sizeof(PON_RogueStatus_t));
				    ret = xmcs_get_rogue_status(&temp) ;
                    COPY_TO_USER((PON_RogueStatus_t *)arg,&temp,sizeof(PON_RogueStatus_t),ret);
				}
				break ;
			case IO_IOG_ONU_MAC :
				{
                   XMCSIF_OnuMac_t temp;
					memset(&temp, 0, sizeof(XMCSIF_OnuMac_t));
				    ret = xmcs_get_onu_mac(&temp) ;
                    COPY_TO_USER((XMCSIF_OnuMac_t *)arg,&temp,sizeof(XMCSIF_OnuMac_t),ret);
				}
				break;
			case IO_IOG_WAN_CNT_STATS:
				ret = xmcs_get_wan_cnt_stats((struct XMCS_WanCntStats_S *)arg);
				break;
			case IO_IOG_STORM_CTL_CONFIG:
				{
                    struct XMCS_StormCtrlConfig_S temp;
					memset(&temp, 0, sizeof(struct XMCS_StormCtrlConfig_S));
					COPY_FROM_USER(&temp,(struct XMCS_StormCtrlConfig_S *)arg,sizeof(struct XMCS_StormCtrlConfig_S),ret);
					ret = xmcs_get_storm_ctrl_config(&temp);
					COPY_TO_USER((struct XMCS_StormCtrlConfig_S *)arg, &temp, sizeof(struct XMCS_StormCtrlConfig_S), ret);	
                }
				break;
			case IO_IOG_DBG_LEVEL : 
				{
                    ushort temp;
					memset(&temp, 0, sizeof(ushort));
					ret = xmcs_get_dbg_level(&temp);
					COPY_TO_USER((ushort *)arg, &temp, sizeof(ushort), ret);	
                }
				break ;
            case IO_IOG_GET_ONLINE_DURATION:
				{	
					ulong temp;
					memset(&temp, 0, sizeof(ulong));
					ret = xmcs_get_pon_online_duration(&temp);
					COPY_TO_USER((ulong*) arg, &temp, sizeof(ulong), ret);
				}
			    break;
#ifdef TCSUPPORT_WAN_GPON
			case IF_IOG_GEMPORT_INFO : 
				{
                    struct XMCS_GemPortInfo_S  temp;
					memset(&temp, 0, sizeof(struct XMCS_GemPortInfo_S ));
					ret = xmcs_get_gem_port_info(&temp);
					COPY_TO_USER((struct XMCS_GemPortInfo_S *)arg, &temp, sizeof(struct XMCS_GemPortInfo_S ), ret);	
                }
				break ;
			case IF_IOG_TCONT_INFO : 
				{
                    struct XMCS_TcontInfo_S  temp;
					memset(&temp, 0, sizeof(struct XMCS_TcontInfo_S ));
					ret = xmcs_get_tcont_info(&temp);
					COPY_TO_USER((struct XMCS_TcontInfo_S *)arg, &temp, sizeof(struct XMCS_TcontInfo_S ), ret);	
                }
				break ;
			case IF_IOG_OMCC_INFO :
				{	
					struct XMCS_OMCC_Info_S  temp;
					memset(&temp, 0, sizeof(struct XMCS_OMCC_Info_S ));
					ret = xmcs_get_omcc_info(&temp) ;
					COPY_TO_USER((struct XMCS_OMCC_Info_S *)arg, &temp, sizeof(struct XMCS_OMCC_Info_S), ret);
				}
				break ;
#endif /* TCSUPPORT_WAN_GPON */
#ifdef TCSUPPORT_WAN_EPON
			case IF_IOG_LLID_INFO : 
				{
					struct XMCS_EponLlidInfo_S temp;
					memset(&temp, 0, sizeof(struct XMCS_EponLlidInfo_S ));
					ret = xmcs_get_llid_info(&temp);
					COPY_TO_USER(((struct XMCS_EponLlidInfo_S *)arg), &temp, sizeof(struct XMCS_EponLlidInfo_S), ret);
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

