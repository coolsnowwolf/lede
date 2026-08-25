/*
* File Name: xmcs_phy.c
* Description: PHY I/O Command Process for xPON Management Control 
*			  Subsystem
*
******************************************************************/
#include <linux/kernel.h>

#include "common/drv_global.h"
#include "xmcs/xmcs_phy.h"
#include "../../inc/common/phy_if_wrapper.h"

/***************************************************************
typedef enum {
	XMCS_PHY_ALARM_TX_POWER_HIGH	= (1<<0),
	XMCS_PHY_ALARM_TX_POWER_LOW		= (1<<1),
	XMCS_PHY_ALARM_TX_CURRENT_HIGH	= (1<<2),
	XMCS_PHY_ALARM_TX_CURRENT_LOW	= (1<<3),
	XMCS_PHY_ALARM_RX_POWER_HIGH	= (1<<4),
	XMCS_PHY_ALARM_RX_POWER_LOW		= (1<<5),
} XMCSPHY_PhyTransAlarm_t
***************************************************************/
void xmcs_detect_phy_trans_alarm(uint alarm)
{
    uint value = 0 ;
    
    PON_MSG(MSG_TRACE, "Send PHY transceiver alarm, %x\n", alarm) ;

    value |= (alarm&PHY_TRANS_TX_HIGH_POWER_ALARM) ? XMCS_PHY_ALARM_TX_POWER_HIGH : 0 ;
    value |= (alarm&PHY_TRANS_TX_LOW_POWER_ALARM) ? XMCS_PHY_ALARM_TX_POWER_LOW : 0 ;
    value |= (alarm&PHY_TRANS_TX_HIGH_CUR_ALARM) ? XMCS_PHY_ALARM_TX_CURRENT_HIGH : 0 ;
    value |= (alarm&PHY_TRANS_TX_LOW_CUR_ALARM) ? XMCS_PHY_ALARM_TX_CURRENT_LOW : 0 ;
    value |= (alarm&PHY_TRANS_RX_HIGH_POWER_ALARM) ? XMCS_PHY_ALARM_RX_POWER_HIGH : 0 ;
    value |= (alarm&PHY_TRANS_RX_LOW_POWER_ALARM) ? XMCS_PHY_ALARM_RX_POWER_LOW : 0 ;
        
    switch(gpPhyData->working_mode){
        case PHY_GPON_CONFIG:
            xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_PHY_TRANS_ALARM, value) ;
            break;
        case PHY_EPON_CONFIG:
            xmcs_report_event(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_PHY_TRANS_ALARM, value) ;
            break;
        default:
            /* do nothing */
            break;
    }
}

/***************************************************************
struct XMCS_PhyRxFecConfig_S {
	XPON_Mode_t		fecMode ;
	XPON_Mode_t		fecStatus ;
	XPON_Mode_t		fecCntReset ;
	struct {
		uint		corrBytes ;
		uint 		corrCodeWords ;
		uint		unCorrCodeWords ;
		uint		totalRxCodeWords ;
		uint		fecSeconds ;
	} fecCounter ;
} ;
***************************************************************/
static int xmcs_set_fec_mode(struct XMCS_PhyRxFecConfig_S *pFecCfg ) 
{
    struct XMCS_PhyRxFecConfig_S tempFecCfg = {0};
    int ret = 0;
    
    if (0 == copy_from_user(&tempFecCfg, pFecCfg, sizeof(tempFecCfg) ) ){
	    ret = XPON_PHY_SET_RX_FEC(((XPON_ENABLE == tempFecCfg.fecMode) ? PHY_ENABLE : PHY_DISABLE) ) ;
        ret = (PHY_SUCCESS == ret) ? 0 : -EINVAL;
         
    }else{
        ret = -EFAULT;
    }

    return ret;
}

/***************************************************************
struct XMCS_PhyRxFecConfig_S {
	XPON_Mode_t		fecMode ;
	XPON_Mode_t		fecStatus ;
	XPON_Mode_t		fecCntReset ;
	struct {
		uint		corrBytes ;
		uint 		corrCodeWords ;
		uint		unCorrCodeWords ;
		uint		totalRxCodeWords ;
		uint		fecSeconds ;
	} fecCounter ;
} ;
***************************************************************/
static int xmcs_get_fec_status(struct XMCS_PhyRxFecConfig_S *pFecCfg) 
{
    struct XMCS_PhyRxFecConfig_S fecCfgTemp = {0} ;
    
	fecCfgTemp.fecMode   = (XPON_PHY_GET(PON_GET_PHY_RX_FEC_GETTING)) ? XPON_ENABLE : XPON_DISABLE ;
	fecCfgTemp.fecStatus = (PHY_TRUE == XPON_PHY_GET(PON_GET_PHY_RX_FEC_STATUS)) ? XPON_ENABLE : XPON_DISABLE ;

    if ( 0 != copy_to_user(pFecCfg, &fecCfgTemp, sizeof(fecCfgTemp ) ) ) {
        return -EFAULT;
    }
		
	return 0 ;
}

/***************************************************************
struct XMCS_PhyRxFecConfig_S {
	XPON_Mode_t		fecMode ;
	XPON_Mode_t		fecStatus ;
	XPON_Mode_t		fecCntReset ;
	struct {
		uint		corrBytes ;
		uint 		corrCodeWords ;
		uint		unCorrCodeWords ;
		uint		totalRxCodeWords ;
		uint		fecSeconds ;
	} fecCounter ;
} ;
***************************************************************/
static int xmcs_set_clear_fec_counter(struct XMCS_PhyRxFecConfig_S *pFecCfg) 
{
	if(pFecCfg->fecCntReset == XPON_ENABLE) {
		XPON_PHY_COUNTER_CLEAR(PHY_ERR_CNT_CLR) ;
	}
	
	return 0 ;
}

/***************************************************************
struct XMCS_PhyRxFecConfig_S {
	XPON_Mode_t		fecMode ;
	XPON_Mode_t		fecStatus ;
	XPON_Mode_t		fecCntReset ;
	struct {
		uint		corrBytes ;
		uint 		corrCodeWords ;
		uint		unCorrCodeWords ;
		uint		totalRxCodeWords ;
		uint		fecSeconds ;
	} fecCounter ;
} ;

typedef struct
{
	uint 			correct_bytes;
	uint 			correct_codewords;
	uint 			uncorrect_codewords;
	uint 			total_rx_codewords;
	uint 			fec_seconds;
}PHY_FecCount_T, *PPHY_FecCount_T;
***************************************************************/
static int xmcs_get_fec_counter(struct XMCS_PhyRxFecConfig_S *pFecCfg) 
{
    struct XMCS_PhyRxFecConfig_S fec_cnt_temp = {0};
    PHY_FecCount_T phyFecCount = {0};

    XPON_PHY_GET_API(PON_GET_PHY_RX_FEC_COUNTER, &phyFecCount);
	fec_cnt_temp.fecCounter.corrBytes = phyFecCount.correct_bytes ;
	fec_cnt_temp.fecCounter.corrCodeWords = phyFecCount.correct_codewords ;
	fec_cnt_temp.fecCounter.unCorrCodeWords = phyFecCount.uncorrect_codewords ;
	fec_cnt_temp.fecCounter.totalRxCodeWords = phyFecCount.total_rx_codewords ;
	fec_cnt_temp.fecCounter.fecSeconds = phyFecCount.fec_seconds ;

    copy_to_user(pFecCfg, &fec_cnt_temp, sizeof(fec_cnt_temp) );
	return 0 ;
}


/***************************************************************
struct XMCS_PhyFrameCount_S {
	XPON_Mode_t		frameCntReset ;
	struct {
		uint		low ;
		uint		high ;
		uint		lof ;
	} frameCounter ;
} ;
***************************************************************/
static int xmcs_set_clear_frame_counter(struct XMCS_PhyFrameCount_S *pFrameCount) 
{
	if(pFrameCount->frameCntReset == PHY_RXFRAME_CNT_CLR) {
		XPON_PHY_COUNTER_CLEAR(PHY_ERR_CNT_CLR) ;
	}
	
	return 0 ;
}

/***************************************************************
struct XMCS_PhyFrameCount_S {
	XPON_Mode_t		frameCntReset ;
	struct {
		uint		low ;
		uint		high ;
		uint		lof ;
	} frameCounter ;
} ;

typedef struct
{
	uint 			frame_count_low;
	uint 			frame_count_high;
	uint 			lof_counter;
}PHY_FrameCount_T, *PPHY_FrameCount_T;
***************************************************************/
static int xmcs_get_frame_counter(struct XMCS_PhyFrameCount_S *pFrameCount) 
{
	PHY_FrameCount_T phyFrameCount = {0} ;
    struct XMCS_PhyFrameCount_S frameCntTemp = {0} ;

    XPON_PHY_GET_API(PON_GET_PHY_RX_FRAME_COUNTER, &phyFrameCount);
	frameCntTemp.frameCounter.low = phyFrameCount.frame_count_low ;
	frameCntTemp.frameCounter.high = phyFrameCount.frame_count_high ;
	frameCntTemp.frameCounter.lof = phyFrameCount.lof_counter ;

    copy_to_user(pFrameCount, &frameCntTemp, sizeof(frameCntTemp) );
	return 0 ;
}

/***************************************************************
struct XMCS_PhyTransSetting_S {
	XPON_Mode_t		txSdInverse ;
	XPON_Mode_t		txFaultInverse ;
	XPON_Mode_t		txBurstEnInverse ;
	XPON_Mode_t		rxSdInverse ;
} ;

typedef struct
{
	unchar	trans_tx_sd_inv_status;
	unchar	trans_burst_en_inv_status;
	unchar	trans_tx_fault_inv_status;
}PHY_TransConfig_T, *PPHY_TransConfig_T;
***************************************************************/
static int xmcs_set_transceiver_config(struct XMCS_PhyTransSetting_S *pTransCfg) 
{
    struct XMCS_PhyTransSetting_S transCfgTemp = {0} ;

	PHY_TransConfig_T phyTransCfg ;
	
    if( 0 != copy_from_user(&transCfgTemp, pTransCfg, sizeof(transCfgTemp) ) ) {
        return -EFAULT;
    }
    
	phyTransCfg.trans_tx_sd_inv_status    = (transCfgTemp.txSdInverse==XPON_ENABLE) ? PHY_ENABLE : PHY_DISABLE ;
	phyTransCfg.trans_burst_en_inv_status = (transCfgTemp.txBurstEnInverse==XPON_ENABLE) ? PHY_ENABLE : PHY_DISABLE ;
	phyTransCfg.trans_tx_fault_inv_status = (transCfgTemp.txFaultInverse==XPON_ENABLE) ? PHY_ENABLE : PHY_DISABLE ;	

	if(PHY_SUCCESS != XPON_PHY_SET_API(PON_SET_PHY_TRANS_TX_SETTINGS, &phyTransCfg ) ) {
		return -EINVAL ;
	}
	
	if(PHY_SUCCESS != XPON_PHY_SET_TRANS_RX((pTransCfg->rxSdInverse==XPON_ENABLE ) ? PHY_ENABLE : PHY_DISABLE ) ) {
		return -EINVAL ;
	}

	return 0 ;
}

/***************************************************************
struct XMCS_PhyTransSetting_S {
	XPON_Mode_t		txSdInverse ;
	XPON_Mode_t		txFaultInverse ;
	XPON_Mode_t		txBurstEnInverse ;
	XPON_Mode_t		rxSdInverse ;
} ;

typedef struct
{
	unchar	trans_tx_sd_inv_status;
	unchar	trans_burst_en_inv_status;
	unchar	trans_tx_fault_inv_status;
}PHY_TransConfig_T, *PPHY_TransConfig_T;
***************************************************************/
static int xmcs_get_transceiver_config(struct XMCS_PhyTransSetting_S *pTransCfg) 
{
	PHY_TransConfig_T phyTransCfg ;
    struct XMCS_PhyTransSetting_S xmcsTransCfgTemp;
    XPON_PHY_GET_API(PON_GET_PHY_TRANS_TX, &phyTransCfg);
	
	xmcsTransCfgTemp.txSdInverse = (phyTransCfg.trans_tx_sd_inv_status==PHY_ENABLE) ? XPON_ENABLE : XPON_DISABLE ;
	xmcsTransCfgTemp.txFaultInverse = (phyTransCfg.trans_tx_fault_inv_status==PHY_ENABLE) ? XPON_ENABLE : XPON_DISABLE ;
	xmcsTransCfgTemp.txBurstEnInverse = (phyTransCfg.trans_burst_en_inv_status==PHY_ENABLE) ? XPON_ENABLE : XPON_DISABLE ;
	xmcsTransCfgTemp.rxSdInverse = (XPON_PHY_GET(PON_GET_PHY_TRANS_RX_GETTING)==PHY_ENABLE) ? XPON_ENABLE : XPON_DISABLE ;

    if(0 != copy_to_user(pTransCfg, &xmcsTransCfgTemp, sizeof(xmcsTransCfgTemp) ) ) {
        return -EFAULT;
    }

	return 0 ;
}

/***************************************************************
struct XMCS_PhyTransParams_S {
	ushort			temperature ;
	ushort			voltage ;
	ushort			txCurrent ;
	ushort			txPower ;
	ushort			rxPower ;
} ;

typedef struct
{
	ushort 			temprature;
	ushort 			supply_voltage;
	ushort 			tx_current;
	ushort 			tx_power;
	ushort 			rx_power;
}PHY_TransParam_T, *PPHY_TransParam_T;
***************************************************************/
static int xmcs_get_transceiver_params(struct XMCS_PhyTransParams_S *pTransParams) 
{
    struct XMCS_PhyTransParams_S tran_paramp_temp;
    
    spin_lock(&gpPhyData->trans_params_lock) ;
	tran_paramp_temp.temperature = gpPhyData->trans_params.temprature       ;
	tran_paramp_temp.voltage     = gpPhyData->trans_params.supply_voltage   ;
	tran_paramp_temp.txCurrent   = gpPhyData->trans_params.tx_current       ;
	tran_paramp_temp.txPower     = gpPhyData->trans_params.tx_power         ;
	tran_paramp_temp.rxPower     = gpPhyData->trans_params.rx_power         ;
    spin_unlock(&gpPhyData->trans_params_lock);
	
    copy_to_user(pTransParams, &tran_paramp_temp, sizeof(tran_paramp_temp) );
	
	return 0 ;
}

/***************************************************************
typedef enum {
	XMCS_PHY_BURST_MODE = 0,
	XMCS_PHY_CONTINUOUS_MODE
} XMCSPHY_TxBurstMode_t ;

struct XMCS_PhyTxBurstCfg_S {
	XMCSPHY_TxBurstMode_t	burstMode ;
} ;
***************************************************************/
static int xmcs_set_burst_config(struct XMCS_PhyTxBurstCfg_S *pBurstCfg) 
{
    struct XMCS_PhyTxBurstCfg_S burstCfgTemp = {0};
    int mode = 0;

    if ( 0 != copy_from_user(&burstCfgTemp, pBurstCfg, sizeof(burstCfgTemp) ) ) {
        return -EFAULT;
    }

    mode = (XMCS_PHY_CONTINUOUS_MODE == burstCfgTemp.burstMode ) ? PHY_TX_CONT_MODE : PHY_TX_BURST_MODE ;
    
	if (PHY_SUCCESS != XPON_PHY_SET_TX_BURST_MODE(mode) ) {
        return -EINVAL;
	}

    return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_get_burst_config(struct XMCS_PhyTxBurstCfg_S *pBurstCfg) 
{
    struct XMCS_PhyTxBurstCfg_S burstCfgTemp = {0};
    int mode = XPON_PHY_GET(PON_GET_PHY_TX_BURST_GETTING) ;
    burstCfgTemp.burstMode = (PHY_TX_CONT_MODE == mode) ? XMCS_PHY_CONTINUOUS_MODE : XMCS_PHY_BURST_MODE ;

    if (0 != copy_to_user(pBurstCfg, &burstCfgTemp, sizeof(burstCfgTemp ) ) ) {
        return -EFAULT;
    }

	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_get_phy_fec_status(struct XMCS_PhyTxRxFecStatus_S* pFecStatus)
{
    struct XMCS_PhyTxRxFecStatus_S fecStatusTemp = {0};
    
    fecStatusTemp.rx_status = XPON_PHY_GET(PON_GET_PHY_RX_FEC_STATUS);
    fecStatusTemp.tx_status = XPON_PHY_GET(PON_GET_PHY_TX_FEC_STATUS);
    
	if(0 != copy_to_user(pFecStatus, &fecStatusTemp, sizeof(fecStatusTemp ) ) ) {
        return -EFAULT;
	}

	return 0;
}

/***************************************************************
***************************************************************/
int phy_cmd_proc(uint cmd, ulong arg) 
{
	int ret ;

	switch(cmd) {
		case PHY_IOS_FEC_MODE :
			ret = xmcs_set_fec_mode((struct XMCS_PhyRxFecConfig_S *)arg) ;
			break ;
		case PHY_IOG_FEC_STATUS :
			ret = xmcs_get_fec_status((struct XMCS_PhyRxFecConfig_S *)arg) ;
			break ;
		case PHY_IOS_FEC_CLEAR :
			ret = xmcs_set_clear_fec_counter((struct XMCS_PhyRxFecConfig_S *)arg) ;
			break ;
		case PHY_IOG_FEC_COUNTER :
			ret = xmcs_get_fec_counter((struct XMCS_PhyRxFecConfig_S *)arg) ;
			break ;
		case PHY_IOS_FRAME_CLEAR :
			ret = xmcs_set_clear_frame_counter((struct XMCS_PhyFrameCount_S *)arg) ;
			break ;
		case PHY_IOG_FRAME_COUNTER :
			ret = xmcs_get_frame_counter((struct XMCS_PhyFrameCount_S *)arg) ;
			break ;
		case PHY_IOS_TRANSCEIVER_CONFIG :
			ret = xmcs_set_transceiver_config((struct XMCS_PhyTransSetting_S *)arg) ;
			break ;
		case PHY_IOG_TRANSCEIVER_CONFIG :
			ret = xmcs_get_transceiver_config((struct XMCS_PhyTransSetting_S *)arg) ;
			break ;
		case PHY_IOG_TRANSCEIVER_PARAMS :
			ret = xmcs_get_transceiver_params((struct XMCS_PhyTransParams_S *)arg) ;
			break ;
		case PHY_IOS_TX_BURST_CONFIG :
			ret = xmcs_set_burst_config((struct XMCS_PhyTxBurstCfg_S *)arg) ;
			break ;	
		case PHY_IOG_TX_BURST_CONFIG :
			ret = xmcs_get_burst_config((struct XMCS_PhyTxBurstCfg_S *)arg) ;
			break ;	
		case PHY_IOG_TX_RX_FEC_STATUS:
			ret = xmcs_get_phy_fec_status((struct XMCS_PhyTxRxFecStatus_S *)arg);
			break;
		default:
			PON_MSG(MSG_ERR, "No such I/O command, cmd: %x\n", cmd) ;
			break ;
	}

	return ret ;
}


