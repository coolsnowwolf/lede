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
#ifndef _ECNT_HOOK_QDMA_H_
#define _ECNT_HOOK_QDMA_H_


/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/	
#include "ecnt_hook.h"
#include "ecnt_hook_qdma_type.h"


/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/


/************************************************************************
*                  M A C R O S
*************************************************************************
*/


/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/

static inline int QDMA_API_INIT(unsigned int mainType, QDMA_InitCfg_t *pInitCfg) {
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_INIT;
	in_data.qdma_private.pInitCfg = pInitCfg;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_DEINIT(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_DEINIT;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_TX_DMA_MODE(unsigned int mainType, QDMA_Mode_t txMode){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_TX_DMA_MODE;
	in_data.qdma_private.mode = txMode ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_RX_DMA_MODE(unsigned int mainType, QDMA_Mode_t rxMode){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_RX_DMA_MODE;
	in_data.qdma_private.mode = rxMode ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_LOOPBACK_MODE(unsigned int mainType, QDMA_LoopbackMode_t lbMode){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_LOOPBACK_MODE;
	in_data.qdma_private.lbMode = lbMode ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_REGISTER_HOOKFUNC(unsigned int mainType, QDMA_CbType_t type, void *pCbFun){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_REGISTER;
	in_data.qdma_private.qdma_reg_cb.type = type ;
	in_data.qdma_private.qdma_reg_cb.pCbFun = pCbFun ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_UNREGISTER_HOOKFUNC(unsigned int mainType, QDMA_CbType_t type){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_UNREGISTER;
	in_data.qdma_private.qdma_reg_cb.type = type ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_ENABLE_RXPKT_INT(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_ENABLE_RXPKT_INT;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_DISABLE_RXPKT_INT(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_DISABLE_RXPKT_INT;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_RECEIVE_PACKETS(unsigned int mainType, int maxPkts){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_RECEIVE_PACKETS;
	in_data.qdma_private.cnt = maxPkts ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_TRANSMIT_PACKETS(unsigned int mainType, struct sk_buff *skb, uint msg0, uint msg1, struct port_info *pMacInfo){
	struct ECNT_QDMA_Data in_data;
	int ret=0;
	QDMA_TxMsg_Ex_T txMsg;

	in_data.function_id = QDMA_FUNCTION_TRANSMIT_PACKETS;
	in_data.qdma_private.qdma_transmit.skb = skb ;
	txMsg.txmsg0 = msg0;
	txMsg.txmsg1 = msg1;
	txMsg.txmsg2 = 0xffff;
	in_data.qdma_private.qdma_transmit.pTxMsg = &txMsg;
	in_data.qdma_private.qdma_transmit.pMacInfo = pMacInfo ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TX_QOS_WEIGHT(unsigned int mainType, QDMA_TxQosWeightType_t weightBase, QDMA_TxQosWeightScale_t weightScale){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TX_QOS_WEIGHT;
	in_data.qdma_private.qdma_tx_qos.weightBase = weightBase ;
	in_data.qdma_private.qdma_tx_qos.weightScale = weightScale ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_TX_QOS_WEIGHT(unsigned int mainType, QDMA_TxQosWeightType_t *pWeightBase, QDMA_TxQosWeightScale_t *pWeightScale){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TX_QOS_WEIGHT;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	*pWeightBase = in_data.qdma_private.qdma_tx_qos.weightBase ;
	*pWeightScale = in_data.qdma_private.qdma_tx_qos.weightScale ;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TX_QOS(unsigned int mainType, QDMA_TxQosScheduler_T *pTxQos){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TX_QOS;
	in_data.qdma_private.qdma_tx_qos.pTxQos = pTxQos ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_TX_QOS(unsigned int mainType, QDMA_TxQosScheduler_T *pTxQos){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TX_QOS;
	in_data.qdma_private.qdma_tx_qos.pTxQos = pTxQos ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_MAC_LIMIT_THRESHOLD(unsigned int mainType, uint chnlThrh, uint queueThrh){
	struct ECNT_QDMA_Data in_data;
	QDMA_MacLimitThrh_T mac_limit_thrh;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_MAC_LIMIT_THRESHOLD;
	mac_limit_thrh.chnlThrh = chnlThrh ;
	mac_limit_thrh.queueThrh = queueThrh ;
	in_data.qdma_private.qdma_mac_limit_thrh = &mac_limit_thrh ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_TRANSMIT_PACKETS_WIFI_FAST(unsigned int mainType, struct sk_buff *skb, int ringIdx){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_TRANSMIT_PACKET_WIFI_FAST;
	in_data.qdma_private.qdma_transmit_wifi_fast.skb = skb ;
	in_data.qdma_private.qdma_transmit_wifi_fast.ringIdx = ringIdx;

	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_DSCP_CHNL_INFO(unsigned int mainType, QDMA_DscpChnlInfo_T *pDscpChnlInfo){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_DSCP_CHNL_INFO;
	in_data.qdma_private.qdma_dscp_chnl_info = pDscpChnlInfo;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TXBUF_THRESHOLD(unsigned int mainType, QDMA_TxBufCtrl_T *pTxBufCtrl){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TXBUF_THRESHOLD;
	in_data.qdma_private.pQdmaTxBufCtrl = pTxBufCtrl ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_TXBUF_THRESHOLD(unsigned int mainType, QDMA_TxBufCtrl_T *pTxBufCtrl){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TXBUF_THRESHOLD;
	in_data.qdma_private.pQdmaTxBufCtrl = pTxBufCtrl ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_PREFETCH_MODE(unsigned int mainType, QDMA_Mode_t prefecthMode, QDMA_Mode_t overDragMode, uint overDragCnt){
	struct ECNT_QDMA_Data in_data;
	QDMA_PrefetchMode_T qdma_prefetch;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_PREFETCH_MODE;
	qdma_prefetch.prefecthMode = prefecthMode ;
	qdma_prefetch.overDragMode = overDragMode ;
	qdma_prefetch.overDragCnt = overDragCnt ;	
	in_data.qdma_private.qdma_prefetch = &qdma_prefetch ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_PKTSIZE_OVERHEAD_EN(unsigned int mainType, QDMA_Mode_t mode){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_PKTSIZE_OVERHEAD_EN;
	in_data.qdma_private.mode = mode ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline QDMA_Mode_t QDMA_API_GET_PKTSIZE_OVERHEAD_EN(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_PKTSIZE_OVERHEAD_EN;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_PKTSIZE_OVERHEAD_VALUE(unsigned int mainType, int cnt){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_PKTSIZE_OVERHEAD_VALUE;
	in_data.qdma_private.cnt = cnt ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline uint QDMA_API_GET_PKTSIZE_OVERHEAD_VALUE(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_PKTSIZE_OVERHEAD_VALUE;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_LMGR_LOW_THRESHOLD(unsigned int mainType, uint lowThrh){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_LMGR_LOW_THRESHOLD;
	in_data.qdma_private.threshold = lowThrh ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline uint QDMA_API_GET_LMGR_LOW_THRESHOLD(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_LMGR_LOW_THRESHOLD;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}


#if defined(TCSUPPORT_CPU_EN7580)
static inline int QDMA_API_ENABLE_RXPKT_INT2(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_ENABLE_RXPKT_INT2;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_DISABLE_RXPKT_INT2(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_DISABLE_RXPKT_INT2;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_RECEIVE_PACKETS_INT2(unsigned int mainType, int maxPkts){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_RECEIVE_PACKETS_INT2;
	in_data.qdma_private.cnt = maxPkts ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_GENERAL_TRTCM_INIT(unsigned int mainType, GENERAL_TrtcmCfg_T *generalTrtcmCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GENERAL_SET_TRTCM_CFG;
	in_data.qdma_private.generalTrtcmCfgPtr = generalTrtcmCfgPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_GENERAL_TRTCM_INIT(unsigned int mainType, GENERAL_TrtcmCfg_T *generalTrtcmCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GENERAL_GET_TRTCM_CFG;
	in_data.qdma_private.generalTrtcmCfgPtr = generalTrtcmCfgPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_GENERAL_RATELIMIT_CFG(unsigned int mainType, GENERAL_TrtcmRatelimitCfg_T *generalTrtcmRatelimitCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GENERAL_SET_RATELIMIT_MODE_CFG;
	in_data.qdma_private.generalTrtcmRatelimitCfgPtr = generalTrtcmRatelimitCfgPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_GENERAL_RATELIMIT_CFG(unsigned int mainType, GENERAL_TrtcmRatelimitCfg_T *generalTrtcmRatelimitCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GENERAL_GET_RATELIMIT_MODE_CFG;
	in_data.qdma_private.generalTrtcmRatelimitCfgPtr = generalTrtcmRatelimitCfgPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_GENERAL_RATELIMIT_VALUE(unsigned int mainType, GENERAL_TrtcmRatelimitSet_T *generalTrtcmRatelimitSetPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GENERAL_SET_RATELIMIT_MODE_VALUE;
	in_data.qdma_private.generalTrtcmRatelimitSetPtr = generalTrtcmRatelimitSetPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_GENERAL_RATELIMIT_VALUE(unsigned int mainType, GENERAL_TrtcmRatelimitSet_T *generalTrtcmRatelimitSetPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GENERAL_GET_RATELIMIT_MODE_VALUE;
	in_data.qdma_private.generalTrtcmRatelimitSetPtr = generalTrtcmRatelimitSetPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_GENERAL_RATELIMIT_BUCKET_SIZE(unsigned int mainType, GENERAL_TrtcmRatelimitSet_T *generalTrtcmRatelimitSetPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GENERAL_SET_RATELIMIT_BUCKET_SIZE;
	in_data.qdma_private.generalTrtcmRatelimitSetPtr = generalTrtcmRatelimitSetPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_GENERAL_TRTCM_CFG(unsigned int mainType, GENERAL_TrtcmCbsPbsCfg_T *generalTrtcmCbsPbsCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GENERAL_SET_TRTCM_MODE_CFG;
	in_data.qdma_private.generalTrtcmCbsPbsCfgPtr = generalTrtcmCbsPbsCfgPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_GENERAL_TRTCM_CFG(unsigned int mainType, GENERAL_TrtcmCbsPbsCfg_T *generalTrtcmCbsPbsCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GENERAL_GET_TRTCM_MODE_CFG;
	in_data.qdma_private.generalTrtcmCbsPbsCfgPtr = generalTrtcmCbsPbsCfgPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_GENERAL_TRTCM_VALUE(unsigned int mainType, GENERAL_TrtcmCbsPbsSet_T *generalTrtcmCbsPbsSetPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GENERAL_SET_TRTCM_MODE_VALUE;
	in_data.qdma_private.generalTrtcmCbsPbsSetPtr = generalTrtcmCbsPbsSetPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_GENERAL_TRTCM_VALUE(unsigned int mainType, GENERAL_TrtcmCbsPbsSet_T *generalTrtcmCbsPbsSetPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GENERAL_GET_TRTCM_MODE_VALUE;
	in_data.qdma_private.generalTrtcmCbsPbsSetPtr = generalTrtcmCbsPbsSetPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_GENERAL_TRTCM_BUCKET_SIZE(unsigned int mainType, GENERAL_TrtcmCbsPbsSet_T *generalTrtcmCbsPbsSetPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GENERAL_SET_TRTCM_BUCKET_SIZE;
	in_data.qdma_private.generalTrtcmCbsPbsSetPtr = generalTrtcmCbsPbsSetPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_FLOW_CNTR_CFG(unsigned int mainType, QDMA_FlowCntIncDropCfg_T *flowCntIncDropCfg){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_FLOW_CNTR_CFG;
	in_data.qdma_private.flowCntIncDropCfg = flowCntIncDropCfg ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_FLOW_CNTR_CFG(unsigned int mainType, QDMA_FlowCntIncDropCfg_T *flowCntIncDropCfg){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_FLOW_CNTR_CFG;
	in_data.qdma_private.flowCntIncDropCfg = flowCntIncDropCfg ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_FLOW_CNTR_VALUE(unsigned int mainType, QDMA_FlowCntValueGet_T *flowCntValueGet){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_FLOW_CNTR_VALUE;
	in_data.qdma_private.flowCntValueGet = flowCntValueGet ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_CLEAR_FLOW_CNTR_VALUE(unsigned int mainType, QDMA_FlowCntValueClear_T *flowCntValueClear){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_CLEAR_FLOW_CNTR_VALUE;
	in_data.qdma_private.flowCntValueClear = flowCntValueClear ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_CHANNEL_CLOSE_STATUS(unsigned int mainType, QDMA_ChannelStatus_T *chnlCloseStatusSet){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_CHANNEL_CLOSE_STATUS;
	in_data.qdma_private.chnlCloseStatusSet = chnlCloseStatusSet ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_CHANNEL_CLOSE_STATUS(unsigned int mainType, QDMA_ChannelStatus_T *chnlCloseStatusGet){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_CHANNEL_CLOSE_STATUS;
	in_data.qdma_private.chnlCloseStatusGet = chnlCloseStatusGet ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_CHANNEL_EMPTY_STATUS(unsigned int mainType, QDMA_ChannelStatus_T *chnlEmptyStatusGet){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_CHANNEL_EMPTY_STATUS;
	in_data.qdma_private.chnlEmptyStatusGet = chnlEmptyStatusGet ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_OAM_MODIFY_FP_EN(unsigned int mainType, QDMA_Mode_t mode){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_OAM_MODIFY_FP_EN;
	in_data.qdma_private.mode = mode ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_OAM_MODIFY_FP_EN(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_OAM_MODIFY_FP_EN;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_MULTICAST_EN(unsigned int mainType, QDMA_Mode_t mode){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_MULTICAST_EN;
	in_data.qdma_private.mode = mode ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_MULTICAST_EN(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_MULTICAST_EN;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_ALLOCATE_METER(unsigned int mainType, QDMA_MeterManage_T *meterManage){
    struct ECNT_QDMA_Data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;

    in_data.function_id = QDMA_FUNCTION_ALLOCATE_METER;
    in_data.qdma_private.meterManage = meterManage ;  
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);   
    if(ret != ECNT_HOOK_ERROR)
    	return in_data.retValue;
    else
    	return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_FREE_METER(unsigned int mainType, int meter_idx){
    struct ECNT_QDMA_Data in_data;
    QDMA_MeterManage_T meterManage;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;
    memset(&meterManage, 0, sizeof(QDMA_MeterManage_T)) ;

    meterManage.meter_id = (meter_idx&0xff);
    meterManage.meter_type = ((meter_idx>>8)&0x3);
    in_data.function_id = QDMA_FUNCTION_FREE_METER;
    in_data.qdma_private.meterManage = &meterManage ;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
    	return in_data.retValue;
    else
    	return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_QOS_FLAG(unsigned int mainType, int *qos_flag_num){
    struct ECNT_QDMA_Data in_data;
    QDMA_MeterManage_T meterManage;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;
    memset(&meterManage, 0, sizeof(QDMA_MeterManage_T)) ;

    in_data.function_id = QDMA_FUNCTION_GET_QOS_FLAG;
    in_data.qdma_private.meterManage = &meterManage ;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    *qos_flag_num = meterManage.Qos_flag;
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_ALLOCATE_ACNT(unsigned int mainType, QDMA_AcntManage_T *acntManage){
    struct ECNT_QDMA_Data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;

    in_data.function_id = QDMA_FUNCTION_ALLOCATE_ACNT;
    in_data.qdma_private.acntManage = acntManage;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);   
    if(ret != ECNT_HOOK_ERROR)
    	return in_data.retValue;
    else
    	return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_FREE_ACNT(unsigned int mainType, QDMA_AcntManage_T *acntManage){
    struct ECNT_QDMA_Data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;

    in_data.function_id = QDMA_FUNCTION_FREE_ACNT;
    in_data.qdma_private.acntManage = acntManage;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);   
    if(ret != ECNT_HOOK_ERROR)
    	return in_data.retValue;
    else
    	return ECNT_HOOK_ERROR;
}

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
static inline int QDMA_API_SET_SLA_CHANNEL_CFG(unsigned int mainType, QDMA_SlaChnlCfg_T *slaChnlCfg){
    struct ECNT_QDMA_Data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;

    in_data.function_id = QDMA_FUNCTION_SET_SLA_CHANNEL_CFG;
    in_data.qdma_private.slaChnlCfgPtr = slaChnlCfg;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);   
    if(ret != ECNT_HOOK_ERROR)
    	return in_data.retValue;
    else
    	return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_SLA_CHANNEL_CFG(unsigned int mainType, QDMA_SlaChnlCfg_T *slaChnlCfg){
    struct ECNT_QDMA_Data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;

    in_data.function_id = QDMA_FUNCTION_GET_SLA_CHANNEL_CFG;
    in_data.qdma_private.slaChnlCfgPtr = slaChnlCfg;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);   
    if(ret != ECNT_HOOK_ERROR)
    	return in_data.retValue;
    else
    	return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_QOS_AGING_CFG(unsigned int mainType, QDMA_QoSAgingCfg_T *qosAgingCfg){
    struct ECNT_QDMA_Data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;

    in_data.function_id = QDMA_FUNCTION_SET_QOS_AGING_CFG;
    in_data.qdma_private.qosAgingCfgPtr = qosAgingCfg;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);   
    if(ret != ECNT_HOOK_ERROR)
    	return in_data.retValue;
    else
    	return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_QOS_AGING_CFG(unsigned int mainType, QDMA_QoSAgingCfg_T *qosAgingCfg){
    struct ECNT_QDMA_Data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;

    in_data.function_id = QDMA_FUNCTION_GET_QOS_AGING_CFG;
    in_data.qdma_private.qosAgingCfgPtr = qosAgingCfg;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);   
    if(ret != ECNT_HOOK_ERROR)
    	return in_data.retValue;
    else
    	return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_PER_QUEUE_AGING_CFG(unsigned int mainType, QDMA_TxQAgingCfg_T *txqAgingCfg){
    struct ECNT_QDMA_Data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;

    in_data.function_id = QDMA_FUNCTION_SET_PER_QUEUE_AGING_CFG;
    in_data.qdma_private.txqAgingCfgPtr = txqAgingCfg;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);   
    if(ret != ECNT_HOOK_ERROR)
    	return in_data.retValue;
    else
    	return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_PER_QUEUE_AGING_CFG(unsigned int mainType, QDMA_TxQAgingCfg_T *txqAgingCfg){
    struct ECNT_QDMA_Data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;

    in_data.function_id = QDMA_FUNCTION_GET_PER_QUEUE_AGING_CFG;
    in_data.qdma_private.txqAgingCfgPtr = txqAgingCfg;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);   
    if(ret != ECNT_HOOK_ERROR)
    	return in_data.retValue;
    else
    	return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_MULTICAST_SPTAG_CFG(unsigned int mainType, QDMA_MulticastSptagCfg_T *multicastSptagCfg){
    struct ECNT_QDMA_Data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;

    in_data.function_id = QDMA_FUNCTION_SET_MULTICAST_SPTAG_CFG;
    in_data.qdma_private.multicastSptagCfgPtr = multicastSptagCfg;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);   
    if(ret != ECNT_HOOK_ERROR)
    	return in_data.retValue;
    else
    	return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_MULTICAST_SPTAG_CFG(unsigned int mainType, QDMA_MulticastSptagCfg_T *multicastSptagCfg){
    struct ECNT_QDMA_Data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;

    in_data.function_id = QDMA_FUNCTION_GET_MULTICAST_SPTAG_CFG;
    in_data.qdma_private.multicastSptagCfgPtr = multicastSptagCfg;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);   
    if(ret != ECNT_HOOK_ERROR)
    	return in_data.retValue;
    else
    	return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_MULTICAST_FPORT_CFG(unsigned int mainType, QDMA_MulticastFportCfg_T *multicastFportCfg){
    struct ECNT_QDMA_Data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;

    in_data.function_id = QDMA_FUNCTION_SET_MULTICAST_FPORT_CFG;
    in_data.qdma_private.multicastFportCfgPtr = multicastFportCfg;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);   
    if(ret != ECNT_HOOK_ERROR)
    	return in_data.retValue;
    else
    	return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_MULTICAST_FPORT_CFG(unsigned int mainType, QDMA_MulticastFportCfg_T *multicastFportCfg){
    struct ECNT_QDMA_Data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;

    in_data.function_id = QDMA_FUNCTION_GET_MULTICAST_FPORT_CFG;
    in_data.qdma_private.multicastFportCfgPtr = multicastFportCfg;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);   
    if(ret != ECNT_HOOK_ERROR)
    	return in_data.retValue;
    else
    	return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TXQ_STATIC_CHANNEL_EN(unsigned int mainType, QDMA_TxQStaticCfg_T *staticThrhCfg){
    struct ECNT_QDMA_Data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;

    in_data.function_id = QDMA_FUNCTION_SET_TXQ_STATIC_CHANNEL_EN;
    in_data.qdma_private.staticThrhCfgPtr = staticThrhCfg;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);   
    if(ret != ECNT_HOOK_ERROR)
    	return in_data.retValue;
    else
    	return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TXQ_STATIC_QUEUE_RATIO(unsigned int mainType, QDMA_TxQStaticThrRatio_T *staticThrhRatio){
    struct ECNT_QDMA_Data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;

    in_data.function_id = QDMA_FUNCTION_SET_TXQ_STATIC_QUEUE_RATIO;
    in_data.qdma_private.staticThrhRatioPtr = staticThrhRatio;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);   
    if(ret != ECNT_HOOK_ERROR)
    	return in_data.retValue;
    else
    	return ECNT_HOOK_ERROR;
}



static inline int QDMA_API_GET_HQOS_EN(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_HQOS_EN;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}
#else
static inline int QDMA_API_GET_HQOS_EN(unsigned int mainType){
	return 0;
}

#endif

#else
static inline int QDMA_API_SET_GENERAL_RATELIMIT_CFG(unsigned int mainType, GENERAL_TrtcmRatelimitCfg_T *generalTrtcmRatelimitCfgPtr){
    return ECNT_CONTINUE;
}

static inline int QDMA_API_GET_GENERAL_RATELIMIT_VALUE(unsigned int mainType, GENERAL_TrtcmRatelimitSet_T *generalTrtcmRatelimitSetPtr){
	return ECNT_CONTINUE;
}

static inline int QDMA_API_SET_GENERAL_RATELIMIT_VALUE(unsigned int mainType, GENERAL_TrtcmRatelimitSet_T *generalTrtcmRatelimitSetPtr){
	return ECNT_CONTINUE;
}

static inline int QDMA_API_GET_HQOS_EN(unsigned int mainType){
	return 0;
}

#endif

static inline int QDMA_API_SET_MULTICAST_1TON_CFG(unsigned int mainType){
    struct ECNT_QDMA_Data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;

    in_data.function_id = QDMA_FUNCTION_SET_MULTICAST_1TON_CFG;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);   
    if(ret != ECNT_HOOK_ERROR)
    	return in_data.retValue;
    else
    	return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_CHANNEL_CFG(unsigned int mainType, unsigned int channel){
    struct ECNT_QDMA_Data in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(struct ECNT_QDMA_Data)) ;

    in_data.function_id = QDMA_FUNCTION_GET_CHANNEL_CFG;
	in_data.qdma_private.channel= channel;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);   
    if(ret != ECNT_HOOK_ERROR)
    	return in_data.retValue;
    else
    	return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TX_DROP_EN(unsigned int mainType, QDMA_Mode_t mode){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TX_DROP_EN;
	in_data.qdma_private.mode = mode ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_TX_DROP_EN(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TX_DROP_EN;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TX_RATEMETER(unsigned int mainType, QDMA_TxRateMeter_T *txRateMeterPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TX_RATEMETER;
	in_data.qdma_private.txRateMeterPtr = txRateMeterPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_TX_RATEMETER(unsigned int mainType, QDMA_TxRateMeter_T *txRateMeterPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TX_RATEMETER;
	in_data.qdma_private.txRateMeterPtr = txRateMeterPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_ENABLE_TX_RATELIMIT(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_ENABLE_TX_RATELIMIT;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

#if 0
static inline int QDMA_API_SET_TX_RATELIMIT_CFG(unsigned int mainType, QDMA_TxRateLimitCfg_T *txRateLimitCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TX_RATELIMIT_CFG;
	in_data.qdma_private.txRateLimitCfgPtr = txRateLimitCfgPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_TX_RATELIMIT_CFG(unsigned int mainType, QDMA_TxRateLimitCfg_T *txRateLimitCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TX_RATELIMIT_CFG;
	in_data.qdma_private.txRateLimitCfgPtr = txRateLimitCfgPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}
#endif

static inline int QDMA_API_SET_TX_RATELIMIT(unsigned int mainType, QDMA_TxRateLimitSet_T *txRateLimitSetPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TX_RATELIMIT;
	in_data.qdma_private.txRateLimitSetPtr = txRateLimitSetPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_TX_RATELIMIT(unsigned int mainType, QDMA_TxRateLimitGet_T *txRateLimitGetPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TX_RATELIMIT;
	in_data.qdma_private.txRateLimitGetPtr = txRateLimitGetPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TX_DBA_REPORT(unsigned int mainType, QDMA_TxDbaReport_T *txDbaReportPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TX_DBAREPORT;
	in_data.qdma_private.txDbaReportPtr = txDbaReportPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_TX_DBA_REPORT(unsigned int mainType, QDMA_TxDbaReport_T *txDbaReportPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TX_DBAREPORT;
	in_data.qdma_private.txDbaReportPtr = txDbaReportPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_RX_PROTECT_EN(unsigned int mainType, QDMA_Mode_t mode){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_RX_PROTECT_EN;
	in_data.qdma_private.mode = mode ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_RX_PROTECT_EN(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_RX_PROTECT_EN;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_RX_LOW_THRESHOLD(unsigned int mainType, QDMA_RxLowThreshold_T *rxLowThrhPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_RX_LOW_THRESHOLD;
	in_data.qdma_private.rxLowThresholdPtr = rxLowThrhPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_RX_LOW_THRESHOLD(unsigned int mainType, QDMA_RxLowThreshold_T *rxLowThrhPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_RX_LOW_THRESHOLD;
	in_data.qdma_private.rxLowThresholdPtr = rxLowThrhPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_RX_RATELIMIT_EN(unsigned int mainType, QDMA_Mode_t mode){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_RX_RATELIMIT_EN;
	in_data.qdma_private.mode = mode ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_RX_RATELIMIT_PKT_MODE(unsigned int mainType, QDMA_RxPktMode_t pktMode )
{
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_RX_RATELIMIT_PKT_MODE;
	in_data.qdma_private.pktMode = pktMode ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_RX_RATELIMIT_CFG(unsigned int mainType, QDMA_RxRateLimitCfg_T *rxRateLimitCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_RX_RATELIMIT_CFG;
	in_data.qdma_private.rxRateLimitCfgPtr = rxRateLimitCfgPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_RX_RATELIMIT(unsigned int mainType, QDMA_RxRateLimitSet_T *rxRateLimitSetPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_RX_RATELIMIT;
	in_data.qdma_private.rxRateLimitSetPtr = rxRateLimitSetPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_RX_RATELIMIT(unsigned int mainType, QDMA_RxRateLimitGet_T *rxRateLimitGetPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_RX_RATELIMIT;
	in_data.qdma_private.rxRateLimitGetPtr = rxRateLimitGetPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TXQ_DEI_DROP_MODE(unsigned int mainType, QDMA_Mode_t deiDropMode){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TXQ_DEI_DROP_MODE;
	in_data.qdma_private.mode = deiDropMode ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline QDMA_Mode_t QDMA_API_GET_TXQ_DEI_DROP_MODE(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TXQ_DEI_DROP_MODE;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TXQ_CNGST_MODE(unsigned int mainType, QDMA_Mode_t dynCngstEn){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TXQ_CNGST_MODE;
	in_data.qdma_private.mode = dynCngstEn ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline QDMA_Mode_t QDMA_API_GET_TXQ_CNGST_MODE(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TXQ_CNGST_MODE;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TXQ_DEI_THRESHOLD_SCALE(unsigned int mainType, QDMA_TxDynCngstDeiThrhScale_t deiThrhScale){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TXQ_DEI_THRH_SCALE;
	in_data.qdma_private.threshold = deiThrhScale ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline QDMA_TxDynCngstDeiThrhScale_t QDMA_API_GET_TXQ_DEI_THRESHOLD_SCALE(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TXQ_DEI_THRH_SCALE;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TXQ_CNGST_AUTO_CONFIG(unsigned int mainType, QDMA_txCngstCfg_t *pTxCngstCfg){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TXQ_CNGST_AUTO_CONFIG;
	in_data.qdma_private.pTxCngstCfg = pTxCngstCfg ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_TXQ_CNGST_AUTO_CONFIG(unsigned int mainType, QDMA_txCngstCfg_t *pTxCngstCfg){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TXQ_CNGST_AUTO_CONFIG;
	in_data.qdma_private.pTxCngstCfg = pTxCngstCfg ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TXQ_CNGST_DYNAMIC_THRESHOLD(unsigned int mainType, QDMA_TxQDynCngstThrh_T *txqCngstThrhPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TXQ_CNGST_DYNAMIC_THRESHOLD;
	in_data.qdma_private.dynCngstThrhPtr = txqCngstThrhPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_TXQ_CNGST_DYNAMIC_THRESHOLD(unsigned int mainType, QDMA_TxQDynCngstThrh_T *txqCngstThrhPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TXQ_CNGST_DYNAMIC_THRESHOLD;
	in_data.qdma_private.dynCngstThrhPtr = txqCngstThrhPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TXQ_CNGST_TOTAL_THRESHOLD(unsigned int mainType, QDMA_TxQDynCngstTotalThrh_T *totalThrhPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TXQ_CNGST_TOTAL_THRESHOLD;
	in_data.qdma_private.totalThrhPtr = totalThrhPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_TXQ_CNGST_TOTAL_THRESHOLD(unsigned int mainType, QDMA_TxQDynCngstTotalThrh_T *totalThrhPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TXQ_CNGST_TOTAL_THRESHOLD;
	in_data.qdma_private.totalThrhPtr = totalThrhPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TXQ_CNGST_CHANNEL_THRESHOLD(unsigned int mainType, QDMA_TxQDynCngstChnlThrh_T *chnlThrhPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TXQ_CNGST_CHANNEL_THRESHOLD;
	in_data.qdma_private.chnlThrhPtr = chnlThrhPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_TXQ_CNGST_CHANNEL_THRESHOLD(unsigned int mainType, QDMA_TxQDynCngstChnlThrh_T *chnlThrhPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TXQ_CNGST_CHANNEL_THRESHOLD;
	in_data.qdma_private.chnlThrhPtr = chnlThrhPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TXQ_CNGST_QUEUE_THRESHOLD(unsigned int mainType, QDMA_TxQDynCngstQueueThrh_T *queueThrhPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TXQ_CNGST_QUEUE_THRESHOLD;
	in_data.qdma_private.queueThrhPtr = queueThrhPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_TXQ_CNGST_QUEUE_THRESHOLD(unsigned int mainType, QDMA_TxQDynCngstQueueThrh_T *queueThrhPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TXQ_CNGST_QUEUE_THRESHOLD;
	in_data.qdma_private.queueThrhPtr = queueThrhPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TXQ_CNGST_QUEUE_NONBLOCKING(unsigned int mainType, QDMA_TxQCngstQueueCfg_T *txqCngstQueueCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TXQ_CNGST_QUEUE_NONBLOCKING;
	in_data.qdma_private.txqCngstQueueCfgPtr = txqCngstQueueCfgPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_TXQ_CNGST_QUEUE_NONBLOCKING(unsigned int mainType, QDMA_TxQCngstQueueCfg_T *txqCngstQueueCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TXQ_CNGST_QUEUE_NONBLOCKING;
	in_data.qdma_private.txqCngstQueueCfgPtr = txqCngstQueueCfgPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TXQ_CNGST_CHANNEL_NONBLOCKING(unsigned int mainType, QDMA_TxQCngstChannelCfg_T *txqCngstChannelCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TXQ_CNGST_CHANNEL_NONBLOCKING;
	in_data.qdma_private.txqCngstChannelCfgPtr = txqCngstChannelCfgPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_TXQ_CNGST_CHANNEL_NONBLOCKING(unsigned int mainType, QDMA_TxQCngstChannelCfg_T *txqCngstChannelCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TXQ_CNGST_CHANNEL_NONBLOCKING;
	in_data.qdma_private.txqCngstChannelCfgPtr = txqCngstChannelCfgPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}


static inline int QDMA_API_SET_TXQ_PEEKRATE_PARAMS(unsigned int mainType, QDMA_PeekRateCfg_t *peekrateCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_PEEKRATE_PARAMS;
	in_data.qdma_private.peekrateCfgPtr = peekrateCfgPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_TXQ_PEEKRATE_PARAMS(unsigned int mainType, QDMA_PeekRateCfg_t *peekrateCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_PEEKRATE_PARAMS;
	in_data.qdma_private.peekrateCfgPtr = peekrateCfgPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TXQ_STATIC_QUEUE_NORMAL_THRESHOLD(unsigned int mainType, QDMA_TxQStaticNormalCfg_T *normThrhPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TXQ_STATIC_QUEUE_NORMAL_THRESHOLD;
	in_data.qdma_private.normThrhPtr = normThrhPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_TXQ_STATIC_QUEUE_DEI_THRESHOLD(unsigned int mainType, QDMA_TxQStaticDeiCfg_T *deiThrhPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TXQ_STATIC_QUEUE_DEI_THRESHOLD;
	in_data.qdma_private.deiThrhPtr = deiThrhPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_TXQ_CNGST_DYNAMIC_INFO(unsigned int mainType, QDMA_TxQDynamicCngstInfo_T *allThrhPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TXQ_CNGST_DYNAMIC_INFO;
	in_data.qdma_private.dynCfgPtr = allThrhPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_TXQ_CNGST_STATIC_INFO(unsigned int mainType, QDMA_TxQStaticCngstInfo_T *staticInfoPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_TXQ_CNGST_STATIC_INFO;
	in_data.qdma_private.staticCfgPtr = staticInfoPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_VIRTUAL_CHANNEL_MODE(unsigned int mainType, QDMA_VirtualChannelMode_T *virChnlModePtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_VIRTUAL_CHANNEL_MODE;
	in_data.qdma_private.virChnlModePtr = virChnlModePtr;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_VIRTUAL_CHANNEL_MODE(unsigned int mainType, QDMA_VirtualChannelMode_T *virChnlModePtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_VIRTUAL_CHANNEL_MODE;
	in_data.qdma_private.virChnlModePtr = virChnlModePtr;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_VIRTUAL_CHANNEL_QOS(unsigned int mainType, QDMA_VirtualChannelQoS_T *virChnlQoSPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_VIRTUAL_CHANNEL_QOS;
	in_data.qdma_private.virChnlQoSPtr = virChnlQoSPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_VIRTUAL_CHANNEL_QOS(unsigned int mainType, QDMA_VirtualChannelQoS_T *virChnlQoSPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_VIRTUAL_CHANNEL_QOS;
	in_data.qdma_private.virChnlQoSPtr = virChnlQoSPtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_READ_VIP_INFO(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;
	in_data.function_id = QDMA_FUNCTION_READ_VIP_INFO;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_SET_CNTR_CHANNEL(unsigned int mainType,unsigned channel_id){
	struct ECNT_QDMA_Data in_data;
	int ret=0;
	in_data.function_id = QDMA_FUNCTION_SET_CHANNEL;
	in_data.qdma_private.channel = channel_id;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
} 

static inline int QDMA_API_DUMP_CNTR_CHANNEL(unsigned int mainType,unsigned channel_id){
	struct ECNT_QDMA_Data in_data;
	int ret=0;
	in_data.function_id = QDMA_FUNCTION_DUMP_CHANNEL;
	in_data.qdma_private.channel = channel_id;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
} 

static inline int QDMA_API_SET_MAC_QOS_CONFIG(unsigned int mainType, QDMA_MacQoSCfg_T *macQosCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;
	in_data.function_id = QDMA_FUNCTION_SET_MAC_QOS_CONFIG;
	in_data.qdma_private.macQosCfg = macQosCfgPtr;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
} 

static inline int QDMA_API_RESET(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;
	in_data.function_id = QDMA_FUNCTION_RESET;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_DBG_CNTR_ALL_QUEUE_COUNTER(unsigned int mainType, QDMA_TxQCngstAllQueueValue_T *txqCngstAllQueueValuePtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;
	in_data.function_id = QDMA_FUNCTION_GET_ALL_QUEUE_DBGCNTR;
	in_data.qdma_private.txqCngstAllQueueValuePtr = txqCngstAllQueueValuePtr ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_DRAM_TEST_DMA_CONFIG(unsigned int mainType, QDMA_DramTestCfg_T *dramTestCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_DRAM_TEST_DMA_CONFIG;
	in_data.qdma_private.dramTestCfgPtr = dramTestCfgPtr;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
} 

static inline int QDMA_API_DRAM_TEST_DMA_ENABLE(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_DRAM_TEST_DMA_ENABLE;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
} 

static inline int QDMA_API_DRAM_TEST_RX_DONE(unsigned int mainType, QDMA_DramTestCfg_T *dramTestCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_DRAM_TEST_RX_DONE;
	in_data.qdma_private.dramTestCfgPtr = dramTestCfgPtr;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return dramTestCfgPtr->rxDone;
	else
		return ECNT_HOOK_ERROR;
} 

static inline int QDMA_API_DRAM_TEST_DST_GET(unsigned int mainType, QDMA_DramTestCfg_T *dramTestCfgPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_DRAM_TEST_DST_GET;
	in_data.qdma_private.dramTestCfgPtr = dramTestCfgPtr;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
} 


static inline int QDMA_API_GET_DBG_CNTR_RX_RING_COUNTER(unsigned int mainType, QDMA_RX_Ring_Cnt_T *rxRingPtr){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	if(QDMA_FUNCTION_DUMP_TX_QOS - QDMA_FUNCTION_GET_ALL_QUEUE_DBGCNTR < 2)
	{
		//printk("Not Implement QDMA_FUNCTION_GET_RX_RING_CNT in QDMA\n");
		return ECNT_HOOK_ERROR;
	}

	//in_data.function_id = QDMA_FUNCTION_GET_RX_RING_CNT;
	in_data.function_id = QDMA_FUNCTION_GET_ALL_QUEUE_DBGCNTR+1;
	in_data.qdma_private.rxRingCntPtr = rxRingPtr;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
} 

static inline int QDMA_API_SET_TXMSG(unsigned int mainType, QDMA_TxMsg_Ex_T* pTxMsg,QDMA_SET_TXMSG_FIELD_t field,unsigned int value){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_TXMSG;
	in_data.qdma_private.qdmaSetTxMsgCfg.pTxMsg = pTxMsg;
	in_data.qdma_private.qdmaSetTxMsgCfg.field = field;
	in_data.qdma_private.qdmaSetTxMsgCfg.value = value;

	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
} 

static inline int QDMA_API_TRANSMIT_PACKETS_FULLMSG(unsigned int mainType, struct sk_buff *skb,QDMA_TxMsg_Ex_T* pTxMsg, struct port_info *pMacInfo){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_TRANSMIT_PACKETS;
	in_data.qdma_private.qdma_transmit.skb = skb ;
	in_data.qdma_private.qdma_transmit.pTxMsg = pTxMsg ;
	in_data.qdma_private.qdma_transmit.pMacInfo = pMacInfo ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int QDMA_API_GET_RXMSG(unsigned int mainType, void* pRxMsg,QDMA_GET_RXMSG_FIELD_t field,unsigned int* pValue){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_RXMSG;
	in_data.qdma_private.qdmaGetRxMsgCfg.pRxMsg = pRxMsg;
	in_data.qdma_private.qdmaGetRxMsgCfg.field = field;
	in_data.qdma_private.qdmaGetRxMsgCfg.pValue = pValue;

	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
} 

static inline int QDMA_API_GET_RXMSG_ETHER(unsigned int mainType, void* pRxMsg,QDMA_RxMsg_Ether_T* p_rxmsg_eth){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_RXMSG_ETH;
	in_data.qdma_private.qdmaGetRxMsgCfgEth.pRxMsg = pRxMsg;
	in_data.qdma_private.qdmaGetRxMsgCfgEth.p_rxmsg_eth = p_rxmsg_eth;

	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
} 

static inline int QDMA_API_CHECK_DSCP_IS_FREE(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_CHECK_DSCP_IS_FREE;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
} 

static inline int QDMA_API_SET_DOWNSTREAM_QOS_MODE(unsigned int mainType){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_SET_DOWNSTREAM_QOS;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
} 

static inline int QDMA_API_GET_DOWNSTREAM_QOS_MODE(unsigned int mainType, QDMA_Mode_t dnStreamQosMode){
	struct ECNT_QDMA_Data in_data;
	int ret=0;

	in_data.function_id = QDMA_FUNCTION_GET_DOWNSTREAM_QOS;
	in_data.qdma_private.mode = dnStreamQosMode ;
	ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
} 

/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

#endif /* _ECNT_HOOK_QDMA_H_ */

