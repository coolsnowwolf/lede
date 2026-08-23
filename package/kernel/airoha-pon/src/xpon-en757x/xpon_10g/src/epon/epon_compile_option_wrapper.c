#include <linux/module.h>
#include <linux/kernel.h>

#include "epon/epon.h"
#include "common/xpon_global.h"
#include "epon/epon_act.h"
#include "epon/epon_compile_option_wrapper.h"
#include <ecnt_hook/ecnt_hook_qdma.h>
//#include "qdma_bmgr.h"
#include <ecnt_hook/ecnt_hook_fe.h>
#include "common/xpon_led.h"
#if defined(TCSUPPORT_XPON_LED)
#include <asm/tc3162/ledcetrl.h>
#endif

/*______________________________________________________________________________
**	function name
**		epon_set_qdma_qos
**	description:
**		set qdma qos cfg
**	parameters:
**		None
**	global:
**		gpWanPriv
**	return:
**		0:success
**	call:
**		QDMA_API_SET_TX_RATEMETER
**		QDMA_API_SET_TXQ_CNGST_AUTO_CONFIG
**		QDMA_API_GET_TXQ_CNGST_DYNAMIC_THRESHOLD
**		QDMA_API_SET_TXQ_CNGST_DYNAMIC_THRESHOLD
**		xpon_set_qdma_qos
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_set_qdma_qos(void)
{
#if 0
	QDMA_TxRateMeter_T txRateMeter;
	QDMA_TxQDynCngstThrh_T txqDynCngstThrh;
	QDMA_txCngstCfg_t txCngstCfg;
	
	memset(&txRateMeter, 0, sizeof(QDMA_TxRateMeter_T));
	UNION_IC_FUNCTION_HOOK(PACKETS_ACTION_EPON_QOS_TX_RATE_METER_CFG, NULL, &txRateMeter);

	QDMA_API_SET_TX_RATEMETER(ECNT_QDMA_WAN, &txRateMeter);
	
	memset(&txCngstCfg, 0, sizeof(QDMA_txCngstCfg_t));
	txCngstCfg.txCngstMode = QDMA_TXCNGST_DYNAMIC_PEAKRATE_MARGIN;
	txCngstCfg.peekRateMargin = QDMA_TXCNGST_PEEKRATE_MARGIN_100;
	txCngstCfg.peekRateDuration = 0xff;
	QDMA_API_SET_TXQ_CNGST_AUTO_CONFIG(ECNT_QDMA_WAN, &txCngstCfg);

	memset(&txqDynCngstThrh, 0, sizeof(QDMA_TxQDynCngstThrh_T));
	QDMA_API_GET_TXQ_CNGST_DYNAMIC_THRESHOLD(ECNT_QDMA_WAN, &txqDynCngstThrh);
	txqDynCngstThrh.dynCngstTotalMinThrh = 0x300;
	txqDynCngstThrh.dynCngstChnlMinThrh = 0x30;
	txqDynCngstThrh.dynCngstQueueMinThrh = 0x30;
	QDMA_API_SET_TXQ_CNGST_DYNAMIC_THRESHOLD(ECNT_QDMA_WAN, &txqDynCngstThrh);
#endif
	return 0;
}

/*______________________________________________________________________________
**	function name
**		set_epon_queue_threshold
**	description:
**		set qdma queue threshold
**	parameters:
**		p_queue_threshold
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		epon_set_queue_threshold
**	revision:
**		v1.0
**____________________________________________________________________________*/

int set_epon_queue_threshold(void *p_queue_threshold)
{

	return 	epon_set_queue_threshold((eponQueueThreshold_t *)p_queue_threshold) ;
	
}

/*______________________________________________________________________________
**	function name
**		epon_event_report
**	description:
**		report event 
**	parameters:
**		id
**          val
**	global:
**		None
**	return:
**		None
**	call:
**		xmcs_report_event
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_event_report(__u16 id,__u16 val)
{
	xmcs_report_event(XMCS_EVENT_TYPE_EPON, id, val) ;
}
/*______________________________________________________________________________
**	function name
**		disable_gdma2_and_channel_retire
**	description:
**		disable fe channel
**	parameters:
**		None
**	global:
**		None
**	return:
**		None
**	call:
**		FE_API_SET_CHANNEL_ENABLE
**		FE_API_SET_HWFWD_CHANNEL
**		FE_API_SET_CHANNEL_RETIRE_ALL
**	revision:
**		v1.0
**____________________________________________________________________________*/

void disable_gdma2_and_channel_retire(void)
{    
    unchar i = 0;
	/* disable all tx channel */
	for(i = 0; i < 32; i++) {
		FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, i, FE_DISABLE);
	}
	/* disable all rx channel */
	for(i = 0; i < 16; i++) {
		FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_RX, i, FE_DISABLE);
	}
	/* disable CDM2 rx DMA */
	for(i = 0; i < 8; i++) {
		FE_API_SET_HWFWD_CHANNEL(FE_CDM_SEL_CDMA2, i, FE_DISABLE);
	}
	/* wait for GDM2 finish */
	udelay(1);

	FE_API_SET_CHANNEL_RETIRE_ALL(FE_GDM_SEL_GDMA2, 32);
}
/*______________________________________________________________________________
**	function name
**		enable_gdma2
**	description:
**		enable fe channel
**	parameters:
**		None
**	global:
**		None
**	return:
**		None
**	call:
**		FE_API_SET_CHANNEL_ENABLE
**		FE_API_SET_HWFWD_CHANNEL
**	revision:
**		v1.0
**____________________________________________________________________________*/

void enable_gdma2(void)
{
	unchar i = 0;
    for(i = 0; i < 8; i++) {
        FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, i, FE_ENABLE);
        FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_RX, i, FE_ENABLE);
        FE_API_SET_HWFWD_CHANNEL(FE_CDM_SEL_CDMA2, i, FE_ENABLE);
    }

}
/*______________________________________________________________________________
**	function name
**		epon_olt_nack_led_control
**	description:
**		led control
**	parameters:
**		None
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_olt_nack_led_control(void)
{
#if/*TCSUPPORT_COMPILE*/ !defined(TCSUPPORT_CT_PON_C9)
	change_pon_led_status(LED_OFF);	
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CUC)	
	change_pon_led_status(LED_FLICKER);
#else/*TCSUPPORT_COMPILE*/
#if/*TCSUPPORT_COMPILE*/ !defined(TCSUPPORT_CT_PON_CN_JS)
	change_pon_led_status(LED_OFF);
#endif/*TCSUPPORT_COMPILE*/
#endif/*TCSUPPORT_COMPILE*/
#endif/*TCSUPPORT_COMPILE*/		
}
/*______________________________________________________________________________
**	function name
**		epon_olt_de_register_led_control
**	description:
**		led control
**	parameters:
**		None
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_olt_de_register_led_control(void)
{
#if !defined(TCSUPPORT_CT_PON_CN_JS) || !defined(TCSUPPORT_CUC)
			change_pon_led_status(LED_OFF);	
#endif	
}

