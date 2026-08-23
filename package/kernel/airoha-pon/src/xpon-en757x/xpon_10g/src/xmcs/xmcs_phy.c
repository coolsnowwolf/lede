/*
* File Name: xmcs_phy.c
* Description: PHY I/O Command Process for xPON Management Control 
*			  Subsystem
*
******************************************************************/
#include <linux/kernel.h>

#include "common/xpon_global.h"

#include "xmcs/xmcs_phy.h"
#include "common/phy_if_wrapper.h"


#define I2C_U2_CLK_DIV	(0xc7)
#define PHY_TRANS_REG_TABLE1			0x50
#define PHY_TRANS_VENDOR_NAME			20
#define PHY_TRANS_VENDOR_PN 			40
#define PHY_TRANS_WAVELENGTH 			60
#define PHY_TRANS_VENDOR_SN 			68
  
#define PHY_TRANS_TX_WAVELENGTH        	2

#define XPON_PHY_STATUS_STR_DOWN 		"Down"
#define XPON_PHY_STATUS_STR_UP			"Up"
#define XPON_PHY_STATUS_STR_UNKNOWN		"Unknown"

extern UINT16 SIF_X_Read(UINT8 u1CHannelID, UINT16 u2ClkDiv, UINT8 u1DevAddr, UINT8 u1WordAddrNum,
                         UINT32 u4WordAddr, UINT8 *pu1Buf, UINT16 u2ByteCnt);

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
#if defined(TCSUPPORT_CPU_EN7580)
    fecCfgTemp.fecStatus = (PHY_TRUE == XPON_PHY_GET(PON_GET_PHY_TX_FEC_STATUS)) ? XPON_ENABLE : XPON_DISABLE ;
#else
    fecCfgTemp.fecStatus = (PHY_TRUE == XPON_PHY_GET(PON_GET_PHY_RX_FEC_STATUS)) ? XPON_ENABLE : XPON_DISABLE ;
#endif

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
    if (rdk_gtc_dbg.proc_flag)
    {
        fec_cnt_temp.fecCounter.corrBytes = rdk_gtc_dbg.fecCounter.corrBytes;
        fec_cnt_temp.fecCounter.corrCodeWords = rdk_gtc_dbg.fecCounter.corrCodeWords ;
        fec_cnt_temp.fecCounter.unCorrCodeWords = rdk_gtc_dbg.fecCounter.unCorrCodeWords ;
        fec_cnt_temp.fecCounter.totalRxCodeWords = rdk_gtc_dbg.fecCounter.totalRxCodeWords ;
        rdk_gtc_dbg.proc_flag = 0;
    }

    if(0 != copy_to_user(pFecCfg, &fec_cnt_temp, sizeof(fec_cnt_temp)))
	{
		printk("func:[%s] line:[%d] calling copy_to_user fail \n",__FUNCTION__,__LINE__);
		return -1;
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

    if(0 != copy_to_user(pFrameCount, &frameCntTemp, sizeof(frameCntTemp)))
	{
		printk("func:[%s] line:[%d] calling copy_to_user fail \n",__FUNCTION__,__LINE__);
		return -1;
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
	xmcsTransCfgTemp.txFaultInverse = (phyTransCfg.trans_burst_en_inv_status==PHY_ENABLE) ? XPON_ENABLE : XPON_DISABLE ;
	xmcsTransCfgTemp.txBurstEnInverse = (phyTransCfg.trans_tx_fault_inv_status==PHY_ENABLE) ? XPON_ENABLE : XPON_DISABLE ;
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
static int xmcs_get_transceiver_params(PHY_PARAMS_t *pTransParams) 
{
	PHY_PARAMS_t tran_paramp_temp;
    
    spin_lock_bh(&gpPhyData->trans_params_lock) ;
	tran_paramp_temp.temperature = gpPhyData->trans_params.temprature       ;
	tran_paramp_temp.voltage     = gpPhyData->trans_params.supply_voltage   ;
	tran_paramp_temp.txCurrent   = gpPhyData->trans_params.tx_current       ;
	tran_paramp_temp.txPower     = gpPhyData->trans_params.tx_power         ;
	tran_paramp_temp.rxPower     = gpPhyData->trans_params.rx_power         ;
	 
	memcpy(&tran_paramp_temp.transInfo,&gpPhyData->trans_info,sizeof(PHY_TRANSINFO_T));

    spin_unlock_bh(&gpPhyData->trans_params_lock);
	
    if(0 != copy_to_user(pTransParams, &tran_paramp_temp, sizeof(PHY_PARAMS_t) ))
	{
		printk("func:[%s] line:[%d] calling copy_to_user fail \n",__FUNCTION__,__LINE__);
		return -1;
	}

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
static int xmcs_get_phy_fec_status(PHY_FECSTATUS_t * pFecStatus)
{
    struct XMCS_PhyTxRxFecStatus_S fecStatusTemp = {0};
    
    fecStatusTemp.rx_status = XPON_PHY_GET(PON_GET_PHY_RX_FEC_STATUS);
    fecStatusTemp.tx_status = XPON_PHY_GET(PON_GET_PHY_TX_FEC_STATUS);
    
	if(0 != copy_to_user(pFecStatus, &fecStatusTemp, sizeof(fecStatusTemp ) ) ) {
        return -EFAULT;
	}

	return 0;
}
/*_____________________________________________________________________________
**      function name: xpon_remove_space
**      descriptions:
**           It's used to remove space of the string according to the CUC spec
**      parameters: string,Len
**      global:
**             None
**      return:
**             void
**      call:
**   	         SIF_X_Read
**      revision:
**            None
**____________________________________________________________________________
*/
static int xmcs_get_phy_media_module(phyMedModule_t *pPhyMedModule)
	{
		phyMedModule_t temp;
		memset(&temp, 0, sizeof(phyMedModule_t));
		memcpy(temp.cage, gpGponPriv->gponCfg.phyMedModule.cage, sizeof(gpGponPriv->gponCfg.phyMedModule.cage));
		memcpy(temp.moduleVendor, gpGponPriv->gponCfg.phyMedModule.moduleVendor, sizeof(gpGponPriv->gponCfg.phyMedModule.moduleVendor));
		memcpy(temp.moduleName, gpGponPriv->gponCfg.phyMedModule.moduleName, sizeof(gpGponPriv->gponCfg.phyMedModule.moduleName));
		memcpy(temp.moduleVersion, gpGponPriv->gponCfg.phyMedModule.moduleVersion, sizeof(gpGponPriv->gponCfg.phyMedModule.moduleVersion));
		memcpy(temp.moduleFwVersion, gpGponPriv->gponCfg.phyMedModule.moduleFwVersion, sizeof(gpGponPriv->gponCfg.phyMedModule.moduleFwVersion));
		memcpy(temp.connector, gpGponPriv->gponCfg.phyMedModule.connector, sizeof(gpGponPriv->gponCfg.phyMedModule.connector));
		if(gpPhyData->phy_link_status == PHY_LINK_STATUS_LOS){
			memcpy(temp.status, XPON_PHY_STATUS_STR_DOWN, sizeof(XPON_PHY_STATUS_STR_DOWN));
		}else if(gpPhyData->phy_link_status == PHY_LINK_STATUS_READY){
			memcpy(temp.status, XPON_PHY_STATUS_STR_UP, sizeof(XPON_PHY_STATUS_STR_UP));
		}else
			memcpy(temp.status, XPON_PHY_STATUS_STR_UNKNOWN, sizeof(XPON_PHY_STATUS_STR_UNKNOWN));
		if ( 0 != copy_to_user(pPhyMedModule, &temp, sizeof(temp)) )
		{	
			return -EFAULT;
		}
		return 0;
}

static void xpon_remove_space(char * string,int len)
{
	int i = 0;
	
	if(string == NULL)
		return;
	
	for (i = 0 ; i < len ; i ++){
		if(string[i] == 0x20)
			string[i] = 0;
	}
}
/*_____________________________________________________________________________
**      function name: xpon_I2C_read
**      descriptions:
**           It's used to communicate with optical transceiver by I2c
**      parameters:
**      global:
**             None
**      return:
**             success: 0
**             fail: EINVAL
**      call:
**   	         SIF_X_Read
**      revision:
**            None
**____________________________________________________________________________
*/
UINT16 xpon_I2C_read(UINT8 u1CHannelID, UINT16 u2ClkDiv, UINT8 u1DevAddr, UINT8 u1WordAddrNum,
                         UINT32 u4WordAddr, UINT8 *pu1Buf, UINT16 u2ByteCnt)
{
	uint ret;

	ret = SIF_X_Read(u1CHannelID, u2ClkDiv, u1DevAddr,u1WordAddrNum, u4WordAddr, pu1Buf,u2ByteCnt);
	if(ret == 0) {	
 		PON_MSG(MSG_ERR,"\nError! SIF_X_Read return value = 0, u1DevAddr:0x%x, u4WordAddr:%d\n",u1DevAddr, u4WordAddr);
	}	
	return ret;
}

/*_____________________________________________________________________________
**      function name: xpon_get_transceiver_info
**      descriptions:
**           It's used to get optical transceiver info
**      parameters:
**            see PHY_TRANSINFO_T.
**      global:
**             None
**      return:
**             void
**      call:
**   	         xpon_I2C_read
**      revision:
**            None
**____________________________________________________________________________
*/
void xpon_get_transceiver_info(PHY_TRANSINFO_T *pTransInfo){

	PHY_TRANSINFO_T data;
	
	if(pTransInfo == NULL)
        return ;
	
	memset(&data,0,sizeof(PHY_TRANSINFO_T));
    xpon_I2C_read(0, I2C_U2_CLK_DIV, PHY_TRANS_REG_TABLE1, 1, PHY_TRANS_VENDOR_NAME , data.vendorName, TRANS_VENDOR_NAME_SIZE);
    xpon_I2C_read(0, I2C_U2_CLK_DIV, PHY_TRANS_REG_TABLE1, 1, PHY_TRANS_VENDOR_PN ,   data.vendorPN, TRANS_VENDOR_PN_SIZE);
	xpon_I2C_read(0, I2C_U2_CLK_DIV, PHY_TRANS_REG_TABLE1, 1, PHY_TRANS_VENDOR_SN ,   data.vendorSN, TRANS_VENDOR_SN_SIZE);
	xpon_I2C_read(0, I2C_U2_CLK_DIV, PHY_TRANS_REG_TABLE1, 1, PHY_TRANS_WAVELENGTH ,  (UINT8 *)&data.txWaveLen, PHY_TRANS_TX_WAVELENGTH);
	
	xpon_remove_space(data.vendorName,TRANS_VENDOR_NAME_SIZE);
    xpon_remove_space(data.vendorPN,TRANS_VENDOR_PN_SIZE);
    xpon_remove_space(data.vendorSN,TRANS_VENDOR_SN_SIZE);
	memcpy(pTransInfo,&data,sizeof(PHY_TRANSINFO_T));
}

/***************************************************************
***************************************************************/
int phy_cmd_proc(uint cmd, ulong arg) 
{
	int ret = 0 ;

#ifdef TCSUPPORT_CPU_ARMV8_64
			cmd = cmd & IOCTL_CMD;
#endif

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
			ret = xmcs_get_transceiver_params((PHY_PARAMS_t *)arg) ;
			break ;
		case PHY_IOS_TX_BURST_CONFIG :
			ret = xmcs_set_burst_config((struct XMCS_PhyTxBurstCfg_S *)arg) ;
			break ;	
		case PHY_IOG_TX_BURST_CONFIG :
			ret = xmcs_get_burst_config((struct XMCS_PhyTxBurstCfg_S *)arg) ;
			break ;	
		case PHY_IOG_TX_RX_FEC_STATUS:
			ret = xmcs_get_phy_fec_status((PHY_FECSTATUS_t *)arg);
			break;
		case PHY_IOG_PHY_MED_MODULE:		
			ret = xmcs_get_phy_media_module((phyMedModule_t*)arg);
			break;
		default:
			PON_MSG(MSG_ERR, "No such I/O command, cmd: %x\n", cmd) ;
			break ;
	}

	return ret ;
}


