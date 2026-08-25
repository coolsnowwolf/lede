#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <utime.h>
#include <dirent.h>

#include "common/mgr_pthread.h"
#include "common/mgr_cmd.h"
#include "common/mgr_util.h"

#include "core/fmgr/fmgr.h"
#include "core/cmgr/cmgr.h"
#include "core/dbmgr/dbmgr.h"

/***********************************************************************************
MACRO_CMD_STRUCT(PhyFecConfig)
{
	MGR_Enable_t				RxFecMode ;
	MGR_Enable_t				RxFecStatus ;
} ;

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
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(PhyFecConfig)
{
	MGR_Ret ret = EXEC_ERR_CMD_NOT_SUPPORT ;
	MACRO_CMD_STRUCT(PhyFecConfig) *pObj = &pMsg->u.oPhyFecConfig ;
	struct XMCS_PhyRxFecConfig_S phyFecConfig ;
	
	switch(pMsg->CmdId)
	{
		case MACRO_CMDID_SET(PhyFecConfig) :
			if(pObj->RxFecMode == ENUM_DISABLE) {
				phyFecConfig.fecMode = XPON_DISABLE ;
			} else if(pObj->RxFecMode == ENUM_ENABLE) {
				phyFecConfig.fecMode = XPON_ENABLE ;
			} else {
				ret = EXEC_ERR_PARAM_INVAL ;
				break ;
			} 
			
			if((ret = XMCS_IOCTL_SDI(PHY_IOS_FEC_MODE, &phyFecConfig)) != 0) {
				break ;
			}
			break ;	
		case MACRO_CMDID_GET(PhyFecConfig) :
			if((ret = XMCS_IOCTL_SDI(PHY_IOG_FEC_STATUS, &phyFecConfig)) != 0) {
				break ;
			}
			pObj->RxFecMode = (phyFecConfig.fecMode==XPON_DISABLE) ? ENUM_DISABLE : ENUM_ENABLE ;
			pObj->RxFecStatus = (phyFecConfig.fecStatus==XPON_DISABLE) ? ENUM_DISABLE : ENUM_ENABLE ;
			break ;
 		case MACRO_CMDID_CREATE(PhyFecConfig) :	
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
		case MACRO_CMDID_DELETE(PhyFecConfig) :
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(PhyFecConfig, ret) ;
}


/***********************************************************************************
MACRO_CMD_STRUCT(PhyTransConfig)
{
	MGR_Enable_t				TxSdInverse ;
	MGR_Enable_t				TxFaultInverse ;
	MGR_Enable_t				TxBurstEnInverse ;
	MGR_Enable_t				RxSdInverse ;
} ;

struct XMCS_PhyTransSetting_S {
	XPON_Mode_t		txSdInverse ;
	XPON_Mode_t		txFaultInverse ;
	XPON_Mode_t		txBurstEnInverse ;
	XPON_Mode_t		rxSdInverse ;
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(PhyTransConfig)
{
	MGR_Ret ret = EXEC_ERR_CMD_NOT_SUPPORT ;
	MACRO_CMD_STRUCT(PhyTransConfig) *pObj = &pMsg->u.oPhyTransConfig ;
	struct XMCS_PhyTransSetting_S phyTransConfig ;
	
	switch(pMsg->CmdId)
	{
		case MACRO_CMDID_SET(PhyTransConfig) :
			if(pObj->TxSdInverse == ENUM_DISABLE) {
				phyTransConfig.txSdInverse = XPON_DISABLE ;
			} else if(pObj->TxSdInverse == ENUM_ENABLE) {
				phyTransConfig.txSdInverse = XPON_ENABLE ;
			} else {
				ret = EXEC_ERR_PARAM_INVAL ;
				break ;
			} 
			
			if(pObj->TxFaultInverse == ENUM_DISABLE) {
				phyTransConfig.txFaultInverse = XPON_DISABLE ;
			} else if(pObj->TxFaultInverse == ENUM_ENABLE) {
				phyTransConfig.txFaultInverse = XPON_ENABLE ;
			} else {
				ret = EXEC_ERR_PARAM_INVAL ;
				break ;
			} 

			if(pObj->TxBurstEnInverse == ENUM_DISABLE) {
				phyTransConfig.txBurstEnInverse = XPON_DISABLE ;
			} else if(pObj->TxBurstEnInverse == ENUM_ENABLE) {
				phyTransConfig.txBurstEnInverse = XPON_ENABLE ;
			} else {
				ret = EXEC_ERR_PARAM_INVAL ;
				break ;
			} 

			if(pObj->RxSdInverse == ENUM_DISABLE) {
				phyTransConfig.rxSdInverse = XPON_DISABLE ;
			} else if(pObj->RxSdInverse == ENUM_ENABLE) {
				phyTransConfig.rxSdInverse = XPON_ENABLE ;
			} else {
				ret = EXEC_ERR_PARAM_INVAL ;
				break ;
			} 

			if((ret = XMCS_IOCTL_SDI(PHY_IOS_TRANSCEIVER_CONFIG, &phyTransConfig)) != 0) {
				break ;
			}
			break ;	
		case MACRO_CMDID_GET(PhyTransConfig) :
			if((ret = XMCS_IOCTL_SDI(PHY_IOG_TRANSCEIVER_CONFIG, &phyTransConfig)) != 0) {
				break ;
			}
			pObj->TxSdInverse = (phyTransConfig.txSdInverse==XPON_DISABLE) ? ENUM_DISABLE : ENUM_ENABLE ;
			pObj->TxFaultInverse = (phyTransConfig.txFaultInverse==XPON_DISABLE) ? ENUM_DISABLE : ENUM_ENABLE ;
			pObj->TxBurstEnInverse = (phyTransConfig.txBurstEnInverse==XPON_DISABLE) ? ENUM_DISABLE : ENUM_ENABLE ;
			pObj->RxSdInverse = (phyTransConfig.rxSdInverse==XPON_DISABLE) ? ENUM_DISABLE : ENUM_ENABLE ;
			break ;
 		case MACRO_CMDID_CREATE(PhyTransConfig) :	
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
		case MACRO_CMDID_DELETE(PhyTransConfig) :
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(PhyTransConfig, ret) ;
}

/***********************************************************************************
MACRO_CMD_STRUCT(PhyTransParameters)
{
	ushort					Temperature ;
	ushort					Voltage ;
	ushort					TxCurrent ;
	ushort					TxPower ;
	ushort					RxPower ;
} ;	

struct XMCS_PhyTransParams_S {
	ushort			temperature ;
	ushort			voltage ;
	ushort			txCurrent ;
	ushort			txPower ;
	ushort			rxPower ;
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(PhyTransParameters)
{
	MGR_Ret ret = EXEC_ERR_CMD_NOT_SUPPORT ;
	MACRO_CMD_STRUCT(PhyTransParameters) *pObj = &pMsg->u.oPhyTransParameters ;
	struct XMCS_PhyTransParams_S phyTransParams ;
	
	switch(pMsg->CmdId)
	{
		case MACRO_CMDID_SET(PhyTransParameters) :
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
		case MACRO_CMDID_GET(PhyTransParameters) :
			if((ret = XMCS_IOCTL_SDI(PHY_IOG_TRANSCEIVER_PARAMS, &phyTransParams)) != 0) {
				break ;
			}
			pObj->Temperature = phyTransParams.temperature ;
			pObj->Voltage = phyTransParams.voltage ;
			pObj->TxCurrent = phyTransParams.txCurrent ;
			pObj->TxPower = phyTransParams.txPower ;
			pObj->RxPower = phyTransParams.rxPower ;
			break ;
 		case MACRO_CMDID_CREATE(PhyTransParameters) :	
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
		case MACRO_CMDID_DELETE(PhyTransParameters) :
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(PhyTransParameters, ret) ;
}



/***********************************************************************************
typedef enum {
	ENUM_PHY_BURST_MODE 			= XMCS_PHY_BURST_MODE,
	ENUM_PHY_CONTINUOUS_MODE		= XMCS_PHY_CONTINUOUS_MODE,
} MGR_PhyTxBurstMode_t ;
MACRO_CMD_STRUCT(PhyTxBurstConfig)
{
	MGR_PhyTxBurstMode_t	BurstMode ;
} ;	

struct XMCS_PhyTxBurstCfg_S {
	XMCSPHY_TxBurstMode_t	burstMode ;
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(PhyTxBurstConfig)
{
	MGR_Ret ret = EXEC_ERR_CMD_NOT_SUPPORT ;
	MACRO_CMD_STRUCT(PhyTxBurstConfig) *pObj = &pMsg->u.oPhyTxBurstConfig ;
	struct XMCS_PhyTxBurstCfg_S phyBurstMode ;
	
	switch(pMsg->CmdId)
	{
		case MACRO_CMDID_SET(PhyTxBurstConfig) :
			if(pObj->BurstMode!=ENUM_PHY_BURST_MODE && pObj->BurstMode!=ENUM_PHY_CONTINUOUS_MODE) {
				ret = EXEC_ERR_PARAM_INVAL ;
				break ;
			} 
			
			phyBurstMode.burstMode = pObj->BurstMode ;
			if((ret = XMCS_IOCTL_SDI(PHY_IOS_TX_BURST_CONFIG, &phyBurstMode)) != 0) {
				break ;
			}
			break ;	
		case MACRO_CMDID_GET(PhyTxBurstConfig) :
			if((ret = XMCS_IOCTL_SDI(PHY_IOG_TX_BURST_CONFIG, &phyBurstMode)) != 0) {
				break ;
			}
			pObj->BurstMode = phyBurstMode.burstMode ;
			break ;
 		case MACRO_CMDID_CREATE(PhyTxBurstConfig) :	
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
		case MACRO_CMDID_DELETE(PhyTxBurstConfig) :
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(PhyTxBurstConfig, ret) ;
}

