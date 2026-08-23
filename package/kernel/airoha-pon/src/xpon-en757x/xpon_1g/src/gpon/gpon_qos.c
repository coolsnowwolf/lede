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
* File Name: gpon_qos.c
* Description: 
*
******************************************************************/
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/types.h>

#include "common/drv_global.h"
#include "gpon/gpon.h"

#ifdef TCSUPPORT_CPU_EN7521
#include <ecnt_hook/ecnt_hook_qdma.h>
#endif

/******************************************************************************
******************************************************************************/

#ifdef TCSUPPORT_CPU_EN7521
void gpon_save_cpu_protection(void)
{
	/* LAN cpu protection */
	QDMA_API_GET_RX_RATELIMIT_CFG(ECNT_QDMA_LAN, &(gpGponPriv->gponCfg.lan_rxRateLimitCfg));
	gpGponPriv->gponCfg.lan_rxRateLimit.ringIdx = 0;
	QDMA_API_GET_RX_RATELIMIT(ECNT_QDMA_LAN, &(gpGponPriv->gponCfg.lan_rxRateLimit));

	/* WAN cpu protection */
	QDMA_API_GET_RX_RATELIMIT_CFG(ECNT_QDMA_WAN, &(gpGponPriv->gponCfg.wan_rxRateLimitCfg));
	gpGponPriv->gponCfg.wan_rxRateLimit.ringIdx = 0;
	QDMA_API_GET_RX_RATELIMIT(ECNT_QDMA_WAN, &(gpGponPriv->gponCfg.wan_rxRateLimit));
}

void gpon_enable_cpu_protection(uint pkt_limit)
{
	QDMA_RxPktMode_t pktMode;
	QDMA_RxRateLimitSet_T rxRateLimitSetPtr;

	/* LAN cpu protection */
	pktMode = QDMA_RX_RATE_LIMIT_BY_PACKET;
	QDMA_API_SET_RX_RATELIMIT_PKT_MODE(ECNT_QDMA_LAN, pktMode);

	rxRateLimitSetPtr.ringIdx = 0;
	rxRateLimitSetPtr.rateLimitValue = pkt_limit;
	QDMA_API_SET_RX_RATELIMIT(ECNT_QDMA_LAN, &rxRateLimitSetPtr);

	QDMA_API_SET_RX_RATELIMIT_EN(ECNT_QDMA_LAN, QDMA_ENABLE);

	/* WAN cpu protection */
	pktMode = QDMA_RX_RATE_LIMIT_BY_PACKET;
	QDMA_API_SET_RX_RATELIMIT_PKT_MODE(ECNT_QDMA_WAN, pktMode);

	rxRateLimitSetPtr.ringIdx = 0;
	rxRateLimitSetPtr.rateLimitValue = pkt_limit;
	QDMA_API_SET_RX_RATELIMIT(ECNT_QDMA_WAN, &rxRateLimitSetPtr);

	QDMA_API_SET_RX_RATELIMIT_EN(ECNT_QDMA_WAN, QDMA_ENABLE);
}

void gpon_restore_cpu_protection(void)
{
    QDMA_RxRateLimitSet_T rxRateLimitSetPtr = {0};

	/* LAN cpu protection */
	rxRateLimitSetPtr.ringIdx = gpGponPriv->gponCfg.lan_rxRateLimit.ringIdx;
	rxRateLimitSetPtr.rateLimitValue = gpGponPriv->gponCfg.lan_rxRateLimit.rateLimitValue;
	QDMA_API_SET_RX_RATELIMIT(ECNT_QDMA_LAN, &rxRateLimitSetPtr);
	QDMA_API_SET_RX_RATELIMIT_PKT_MODE(ECNT_QDMA_LAN, gpGponPriv->gponCfg.lan_rxRateLimitCfg.rxRateLimitPktMode);
	QDMA_API_SET_RX_RATELIMIT_EN(ECNT_QDMA_LAN, gpGponPriv->gponCfg.lan_rxRateLimitCfg.rxRateLimitEn);

	/* WAN cpu protection */
	rxRateLimitSetPtr.ringIdx = gpGponPriv->gponCfg.wan_rxRateLimit.ringIdx;
	rxRateLimitSetPtr.rateLimitValue = gpGponPriv->gponCfg.wan_rxRateLimit.rateLimitValue;
	QDMA_API_SET_RX_RATELIMIT(ECNT_QDMA_WAN, &rxRateLimitSetPtr);
	QDMA_API_SET_RX_RATELIMIT_PKT_MODE(ECNT_QDMA_WAN, gpGponPriv->gponCfg.wan_rxRateLimitCfg.rxRateLimitPktMode);
	QDMA_API_SET_RX_RATELIMIT_EN(ECNT_QDMA_WAN, gpGponPriv->gponCfg.wan_rxRateLimitCfg.rxRateLimitEn);
}
#endif

#ifdef TCSUPPORT_ALPHION_PON
unsigned int qos_cnt = 0;
#endif
/******************************************************************************
******************************************************************************/
int gpon_qos_init(void)
{
	int i;
	int ret;

#ifdef TCSUPPORT_CPU_EN7521
	QDMA_TxDbaReport_T gponTrtcmParams ;
#else
	QDMA_TcontTrtcm_T gponTrtcmParams ;

	qdma_set_gpon_trtcm_scale(QDMA_TRTCM_SCALE_1BYTE);
#endif

	if(isAN7552){
		PON_MSG(MSG_TRACE, "Not into qos_init.\n") ;
		return 0;
	}

#ifdef TCSUPPORT_ALPHION_PON
	if(qos_cnt >= 1)
	{
		printk("%s %d qos counter is 1 return\n",__FUNCTION__,__LINE__);
		return 0;
	}
#endif
	for(i=0 ; i<CONFIG_QDMA_CHANNEL ; i++) {
		gponTrtcmParams.channel = i ;
		if((isEN7580 || isEN7523 || isEN7581 || isAN7583 || isAN7552)){
			gponTrtcmParams.cirParamValue = 0xa00000 ;
			gponTrtcmParams.pirParamValue = 0xa00000 ;
		}else{
			gponTrtcmParams.cirParamValue = 0xffff ;
			gponTrtcmParams.pirParamValue = 0xffff ;
		}		
		gponTrtcmParams.cbsParamValue = 0x8000 ;
		gponTrtcmParams.pbsParamValue = 0xffff ;

#ifdef TCSUPPORT_CPU_EN7521
		ret = QDMA_API_SET_TX_DBA_REPORT(ECNT_QDMA_WAN, &gponTrtcmParams);
#else
		ret = qdma_set_gpon_trtcm_params(&gponTrtcmParams);
#endif

		if (ret < 0){
			PON_MSG(MSG_ERR, "failed to set the GPON trTCM parameters.\n") ;
			return -1 ;
		}
	}
	
	/*enable trtcm mode default*/
#ifndef TCSUPPORT_CPU_EN7521
	qdma_set_gpon_trtcm_mode(QDMA_ENABLE);
#endif /* TCSUPPORT_CPU_EN7521 */

#ifdef TCSUPPORT_ALPHION_PON
	qos_cnt++;
#endif
	return 0 ;
}
int gpon_qos_deinit(void) 
{
	int i;
	int ret;
	
#ifdef TCSUPPORT_CPU_EN7521
	QDMA_TxDbaReport_T gponTrtcmParams ;
#ifdef TCSUPPORT_CPU_EN7523
    GENERAL_TrtcmCbsPbsCfg_T generalTrtcmCbsPbsCfg ;
#endif
#else
	QDMA_TcontTrtcm_T gponTrtcmParams ;

	qdma_set_gpon_trtcm_scale(QDMA_TRTCM_SCALE_1BYTE);
#endif

	if(isAN7552){
		PON_MSG(MSG_TRACE, "Not into qos_deinit.\n") ;
		return 0;
	}

#ifdef TCSUPPORT_ALPHION_PON
	if(qos_cnt >= 1)
	{
		printk("%s %d qos counter is 1 return\n",__FUNCTION__,__LINE__);
		return 0;
	}
#endif
	for(i=0 ; i < CONFIG_QDMA_CHANNEL ; i++) {
		gponTrtcmParams.channel = i ;
		gponTrtcmParams.cirParamValue = 0 ;
		gponTrtcmParams.cbsParamValue = 0 ;
		gponTrtcmParams.pirParamValue = 0 ;
		gponTrtcmParams.pbsParamValue = 0 ;

#ifdef TCSUPPORT_CPU_EN7521
		ret = QDMA_API_SET_TX_DBA_REPORT(ECNT_QDMA_WAN, &gponTrtcmParams);
#else
		ret = qdma_set_gpon_trtcm_params(&gponTrtcmParams);
#endif
		if (ret < 0){
			PON_MSG(MSG_ERR, "failed to set the GPON trTCM parameters.\n") ;
			return -1 ;
		}

#ifdef TCSUPPORT_CPU_EN7523
        memset(&generalTrtcmCbsPbsCfg, 0, sizeof(GENERAL_TrtcmCbsPbsCfg_T));
        generalTrtcmCbsPbsCfg.trtcmModule = EGRESS_TRTCM;
        generalTrtcmCbsPbsCfg.Index = i;
        generalTrtcmCbsPbsCfg.CbsMeterEn = GENERAL_METER_DISABLE;
        generalTrtcmCbsPbsCfg.PbsMeterEn = GENERAL_METER_DISABLE;
        ret = QDMA_API_SET_GENERAL_TRTCM_CFG(ECNT_QDMA_WAN, &generalTrtcmCbsPbsCfg);
        if(ret < 0){
            PON_MSG(MSG_ERR, "failed to set the trTCM configuration.\n") ;
			return -1 ;
        }
#endif
	}

	/*disable gpon trtcm mode function*/
#ifndef TCSUPPORT_CPU_EN7521
	qdma_set_gpon_trtcm_mode(QDMA_DISABLE);
#endif

	return 0 ;
}


