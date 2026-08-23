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
	/* LAN cpu protection */
	QDMA_API_SET_RX_RATELIMIT(ECNT_QDMA_LAN, &(gpGponPriv->gponCfg.lan_rxRateLimit));
	QDMA_API_SET_RX_RATELIMIT_PKT_MODE(ECNT_QDMA_LAN, gpGponPriv->gponCfg.lan_rxRateLimitCfg.rxRateLimitPktMode);
	QDMA_API_SET_RX_RATELIMIT_EN(ECNT_QDMA_LAN, gpGponPriv->gponCfg.lan_rxRateLimitCfg.rxRateLimitEn);

	/* WAN cpu protection */
	QDMA_API_SET_RX_RATELIMIT(ECNT_QDMA_WAN, &(gpGponPriv->gponCfg.wan_rxRateLimit));
	QDMA_API_SET_RX_RATELIMIT_PKT_MODE(ECNT_QDMA_LAN, gpGponPriv->gponCfg.wan_rxRateLimitCfg.rxRateLimitPktMode);
	QDMA_API_SET_RX_RATELIMIT_EN(ECNT_QDMA_WAN, gpGponPriv->gponCfg.wan_rxRateLimitCfg.rxRateLimitEn);
}
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

	for(i=0 ; i<CONFIG_QDMA_CHANNEL ; i++) {
		gponTrtcmParams.channel = i ;
		gponTrtcmParams.cirParamValue = 0xffff ;
		gponTrtcmParams.cbsParamValue = 0x8000 ;
		gponTrtcmParams.pirParamValue = 0xffff ;
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

	return 0 ;
}
int gpon_qos_deinit(void) 
{
	int i;
	int ret;
#ifdef TCSUPPORT_CPU_EN7521
	QDMA_TxDbaReport_T gponTrtcmParams ;
#else
	QDMA_TcontTrtcm_T gponTrtcmParams ;

	qdma_set_gpon_trtcm_scale(QDMA_TRTCM_SCALE_1BYTE);
#endif

	for(i=0 ; i < CONFIG_QDMA_CHANNEL ; i++) {
		gponTrtcmParams.channel = i ;
		gponTrtcmParams.cirParamValue = 0 ;
		gponTrtcmParams.pirParamValue = 0 ;

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

	/*disable gpon trtcm mode function*/
#ifndef TCSUPPORT_CPU_EN7521
	qdma_set_gpon_trtcm_mode(QDMA_DISABLE);
#endif

	/*enable green drop function default*/
	xpon_set_qdma_qos(XPON_ENABLE);
	return 0 ;
}


