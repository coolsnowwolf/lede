#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "common/mgr_cmd.h"
#include "common/mgr_pthread.h"
#include "common/mgr_sem.h"
#include "common/mgr_util.h"

#include "core/cmgr/cmgr.h"
#include "core/pmgr/pmgr.h"
#include "core/dbmgr/dbmgr.h"

/***********************************************************************************************
************************************************************************************************
*** AEC Process Function for Performance Management                                          ***
************************************************************************************************
***********************************************************************************************/
/***********************************************************************************
MACRO_CMD_STRUCT(PhyFecCounter)
{
	MGR_U32			CorrBytes ;
	MGR_U32 		CorrCodeWords ;
	MGR_U32			UncorrCodeWords ;
	MGR_U32			TotalRxCodeWords ;
	MGR_U32			FecSeconds ;
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
MACRO_CMD_PROC_FUNCTION(PhyFecCounter)
{
	MGR_Ret ret = EXEC_ERR_CMD_NOT_SUPPORT ;
	MACRO_CMD_STRUCT(PhyFecCounter) *pObj = &pMsg->u.oPhyFecCounter ;
	struct XMCS_PhyRxFecConfig_S phyFecConfig ;
	
	switch(pMsg->CmdId)
	{
		case MACRO_CMDID_SET(PhyFecCounter) :
			phyFecConfig.fecCntReset = XPON_ENABLE ;
			if((ret = XMCS_IOCTL_SDI(PHY_IOS_FEC_CLEAR, &phyFecConfig)) != 0) {
				break ;
			}
			break ;	
		case MACRO_CMDID_GET(PhyFecCounter) :
			if((ret = XMCS_IOCTL_SDI(PHY_IOG_FEC_COUNTER, &phyFecConfig)) != 0) {
				break ;
			}
			pObj->CorrBytes = phyFecConfig.fecCounter.corrBytes ;
			pObj->CorrCodeWords = phyFecConfig.fecCounter.corrCodeWords ;
			pObj->UncorrCodeWords = phyFecConfig.fecCounter.unCorrCodeWords ;
			pObj->TotalRxCodeWords = phyFecConfig.fecCounter.totalRxCodeWords ;
			pObj->FecSeconds = phyFecConfig.fecCounter.fecSeconds ;
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(PhyFecCounter, ret) ;
}


/***********************************************************************************
MACRO_CMD_STRUCT(PhyFrameCounter)
{
	MGR_U32			RxFrameLow ;
	MGR_U32			RxFrameHigh ;
	MGR_U32			RxLosFrame ;
} ;

struct XMCS_PhyFrameCount_S {
	XPON_Mode_t		frameCntReset ;
	struct {
		uint		low ;
		uint		high ;
		uint		lof ;
	} frameCounter ;
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(PhyFrameCounter)
{
	MGR_Ret ret = EXEC_ERR_CMD_NOT_SUPPORT ;
	MACRO_CMD_STRUCT(PhyFrameCounter) *pObj = &pMsg->u.oPhyFrameCounter ;
	struct XMCS_PhyFrameCount_S phyFrameCount ;
	
	switch(pMsg->CmdId)
	{
		case MACRO_CMDID_SET(PhyFrameCounter) :
			phyFrameCount.frameCntReset = XPON_ENABLE ;
			if((ret = XMCS_IOCTL_SDI(PHY_IOS_FRAME_CLEAR, &phyFrameCount)) != 0) {
				break ;
			}
			break ;	
		case MACRO_CMDID_GET(PhyFrameCounter) :
			if((ret = XMCS_IOCTL_SDI(PHY_IOG_FRAME_COUNTER, &phyFrameCount)) != 0) {
				break ;
			}
			pObj->RxFrameLow = phyFrameCount.frameCounter.low ;
			pObj->RxFrameHigh = phyFrameCount.frameCounter.high ;
			pObj->RxLosFrame = phyFrameCount.frameCounter.lof ;
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(PhyFrameCounter, ret) ;
}
