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
#include "common/mgr_sdi.h"

#include "core/fmgr/fmgr.h"
#include "core/cmgr/cmgr.h"
#include "core/dbmgr/dbmgr.h"

#if 0
/***********************************************************************************
MACRO_CMD_STRUCT(GponSnPasswd)
{
	MGR_U8 		sn[CONST_GPON_SN_LENS] ;
	MGR_U8 		passwd[CONST_GPON_PASSWD_LENS] ;
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(GponSnPasswd)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(GponSnPasswd) *pObj = &pMsg->u.oGponSnPasswd ;
	
	switch(pMsg->CmdId) {
		case MACRO_CMDID_CREATE(GponSnPasswd) :		
			break ;
		case MACRO_CMDID_DELETE(GponSnPasswd) :
			break ;
		case MACRO_CMDID_SET(GponSnPasswd) :
			break ;
		case MACRO_CMDID_GET(GponSnPasswd) :
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(GponSnPasswd, ret) ;
}
#endif /* 0 */

/***********************************************************************************
MACRO_CMD_STRUCT(GponSnPasswd)
{
	MGR_U8 		sn[CONST_GPON_SN_LENS] ;
	MGR_U8 		passwd[CONST_GPON_PASSWD_LENS] ;
	MGR_U8      EmergencyState  ;
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(GponSnPasswd)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(GponSnPasswd) *pObj = &pMsg->u.oGponSnPasswd ;
	struct XMCS_GponSnPasswd_S gponSnPasswd ;
	switch(pMsg->CmdId) {
		//case MACRO_CMDID_CREATE(GponSnPasswd) :		
		//	break ;
		//case MACRO_CMDID_DELETE(GponSnPasswd) :		
		//	break ;
		case MACRO_CMDID_SET(GponSnPasswd) :
			memset(&gponSnPasswd, 0, sizeof(struct XMCS_GponSnPasswd_S)) ;
			memcpy(gponSnPasswd.sn, pObj->Serial, GPON_SN_LENS) ;
			memcpy(gponSnPasswd.passwd, pObj->Password, GPON_PASSWD_LENS) ;
			gponSnPasswd.EmergencyState = pObj->EmergencyState;
			gponSnPasswd.PasswdLength = pObj->PasswdLength;
			gponSnPasswd.hexFlag = pObj->hexFlag;
			ret = XMCS_IOCTL_SDI(GPON_IOS_SN_PASSWD, &gponSnPasswd) ;
			break ;
		//case MACRO_CMDID_GET(GponSnPasswd) :		
		//	break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(GponSnPasswd, ret) ;
}

/***********************************************************************************
MACRO_CMD_STRUCT(GponActTimer)
{
	MGR_U32 	to1_timer ;  	// millisecond 
	MGR_U32		to2_timer ;		// millisecond 
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(GponActTimer)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(GponActTimer) *pObj = &pMsg->u.oGponActTimer ;
	struct XMCS_GponActTimer_S gponActTimer ;
	
	switch(pMsg->CmdId) {
		//case MACRO_CMDID_CREATE(GponActTimer) :		
		//	break ;
		//case MACRO_CMDID_DELETE(GponActTimer) :		
		//	break ;
		case MACRO_CMDID_SET(GponActTimer) :
			gponActTimer.to1Timer = pObj->TO1Timer ;
			gponActTimer.to2Timer = pObj->TO2Timer ;
			
			ret = XMCS_IOCTL_SDI(GPON_IOS_ACT_TIMER, &gponActTimer) ;
			break ;
		//case MACRO_CMDID_GET(GponActTimer) :		
		//	break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(GponActTimer, ret) ;
}

/***********************************************************************************
MACRO_CMD_STRUCT(GponSystemInfo)
{
	MGR_U8 				OnuId ;
	MGR_GponState_t 	ActState ;
	MGR_U8 				Serial[CONST_GPON_SN_LENS] ;
	MGR_U8 				Password[CONST_GPON_PASSWD_LENS] ;
	MGR_U8				KeyIdx ;
	MGR_U8				Key[CONST_GPON_ENCRYPT_KEY_LENS] ;
	MGR_U32 			TO1Timer ;  	
	MGR_U32				TO2Timer ;		
	MGR_U16				OMCIChannel ;
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(GponSystemInfo)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(GponSystemInfo) *pObj = &pMsg->u.oGponSystemInfo ;
	struct XMCS_GponOnuInfo_S gponOnuInfo ;
	
	switch(pMsg->CmdId) {
		//case MACRO_CMDID_CREATE(GponSystemInfo) :		
		//	break ;
		//case MACRO_CMDID_DELETE(GponSystemInfo) :		
		//	break ;
		//case MACRO_CMDID_SET(GponSystemInfo) :		
		//	break ;
		case MACRO_CMDID_GET(GponSystemInfo) :
			ret = XMCS_IOCTL_SDI(GPON_IOG_ONU_INFO, &gponOnuInfo) ;
			if(ret == 0) {
				memset(pObj, 0, sizeof(MACRO_CMD_STRUCT(GponSystemInfo))) ;
				
				pObj->OnuId = gponOnuInfo.onuId ;
				if(gponOnuInfo.state == 2) {
					pObj->ActState = ENUM_GPON_STATE_O2 ;
				} else if(gponOnuInfo.state == 3) {
					pObj->ActState = ENUM_GPON_STATE_O3 ;
				} else if(gponOnuInfo.state == 4) {
					pObj->ActState = ENUM_GPON_STATE_O4 ;
				} else if(gponOnuInfo.state == 5) {
					pObj->ActState = ENUM_GPON_STATE_O5 ;
				} else if(gponOnuInfo.state == 6) {
					pObj->ActState = ENUM_GPON_STATE_O6 ;
				} else if(gponOnuInfo.state == 7) {
					pObj->ActState = ENUM_GPON_STATE_O7 ;
				} else {
					pObj->ActState = ENUM_GPON_STATE_O1 ;
				}
				memcpy(pObj->Serial, gponOnuInfo.sn, CONST_GPON_SN_LENS) ;
				memcpy(pObj->Password, gponOnuInfo.passwd, CONST_GPON_PASSWD_LENS) ;
				pObj->PasswdLength=gponOnuInfo.PasswdLength;
				pObj->hexFlag=gponOnuInfo.hexFlag;
				pObj->KeyIdx = gponOnuInfo.keyIdx ;
				memcpy(pObj->Key, gponOnuInfo.key, CONST_GPON_ENCRYPT_KEY_LENS) ;
				pObj->TO1Timer = gponOnuInfo.actTo1Timer ;
				pObj->TO2Timer = gponOnuInfo.actTo2Timer ;
				pObj->OMCIChannel = gponOnuInfo.omcc ;
			}
			break ;		
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(GponSystemInfo, ret) ;
}

/***********************************************************************************
typedef enum {
	ENUM_GPON_TRTCM_SCALE_1B		= XMCS_GPON_TRTCM_SCALE_1B, 
	ENUM_GPON_TRTCM_SCALE_2B		= XMCS_GPON_TRTCM_SCALE_2B,  
	ENUM_GPON_TRTCM_SCALE_4B		= XMCS_GPON_TRTCM_SCALE_4B,  
	ENUM_GPON_TRTCM_SCALE_8B		= XMCS_GPON_TRTCM_SCALE_8B,  
	ENUM_GPON_TRTCM_SCALE_16B		= XMCS_GPON_TRTCM_SCALE_16B, 
	ENUM_GPON_TRTCM_SCALE_32B		= XMCS_GPON_TRTCM_SCALE_32B, 
	ENUM_GPON_TRTCM_SCALE_64B		= XMCS_GPON_TRTCM_SCALE_64B, 
	ENUM_GPON_TRTCM_SCALE_128B		= XMCS_GPON_TRTCM_SCALE_128B,
	ENUM_GPON_TRTCM_SCALE_256B		= XMCS_GPON_TRTCM_SCALE_256B,
	ENUM_GPON_TRTCM_SCALE_512B		= XMCS_GPON_TRTCM_SCALE_512B,
	ENUM_GPON_TRTCM_SCALE_1K		= XMCS_GPON_TRTCM_SCALE_1K,  
	ENUM_GPON_TRTCM_SCALE_2K		= XMCS_GPON_TRTCM_SCALE_2K,  
	ENUM_GPON_TRTCM_SCALE_4K		= XMCS_GPON_TRTCM_SCALE_4K,  
	ENUM_GPON_TRTCM_SCALE_8K		= XMCS_GPON_TRTCM_SCALE_8K,  
	ENUM_GPON_TRTCM_SCALE_16K		= XMCS_GPON_TRTCM_SCALE_16K, 
	ENUM_GPON_TRTCM_SCALE_32K		= XMCS_GPON_TRTCM_SCALE_32K, 
	ENUM_GPON_TRTCM_SCALE_ITEMS     = XMCS_GPON_TRTCM_SCALE_ITEMS
} MGR_GponTrtcmScale_t ;

MACRO_CMD_MASK(GponDbaConfig)
{
	MASK_GponBlockSize					= (1<<0) ,
	MASK_GponTrtcmMode					= (1<<1) ,
	MASK_GponTrtcmScale					= (1<<2) ,
	MASK_GponTrtcmParams				= (1<<3)
} ;

MACRO_CMD_STRUCT(GponDbaConfig)
{
	MGR_U16 				BlockSize ;
	MGR_Enable_t			GponTrtcmMode ;
	MGR_GponTrtcmScale_t	GponTrtcmScale ;
	struct {
		MGR_U8					Channel ;
		MGR_U16					CIRValue ;
		MGR_U16					CBSUnit ;
		MGR_U16					PIRValue ;
		MGR_U16					PBSUnit ;
	} GponTrtcm[CONFIG_GPON_MAX_TCONT] ;
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(GponDbaConfig)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(GponDbaConfig) *pObj = &pMsg->u.oGponDbaConfig ;
	int i ;
	struct XMCS_GponTrtcmConfig_S gponTrtcmConfig ;
	struct XMCS_GponTrtcmParams_S gponTrtcmParams ;
	
	switch(pMsg->CmdId) {
		//case MACRO_CMDID_CREATE(GponDbaConfig) :		
		//	break ;
		//case MACRO_CMDID_DELETE(GponDbaConfig) :		
		//	break ;
		case MACRO_CMDID_SET(GponDbaConfig) :
			if(pMsg->Mask & MASK_GponBlockSize) {
				if((ret = XMCS_IOCTL_SDI(GPON_IOS_DBA_BLOCK_SIZE, pObj->BlockSize)) != 0) {
					break ;
				}
			} 
			if(pMsg->Mask & MASK_GponTrtcmMode) {
				if(pObj->GponTrtcmMode!=ENUM_DISABLE && pObj->GponTrtcmMode!=ENUM_ENABLE) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				gponTrtcmConfig.trtcmMode = (pObj->GponTrtcmMode==ENUM_ENABLE) ? XPON_ENABLE : XPON_DISABLE ;
				if((ret = XMCS_IOCTL_SDI(GPON_IOS_GPON_TRTCM_MODE, &gponTrtcmConfig)) != 0) {
					break ;
				}
			} 
			if(pMsg->Mask & MASK_GponTrtcmScale) {
				if(pObj->GponTrtcmScale<0 || pObj->GponTrtcmScale>=ENUM_GPON_TRTCM_SCALE_ITEMS) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				gponTrtcmConfig.trtcmScale = pObj->GponTrtcmScale ;
				if((ret = XMCS_IOCTL_SDI(GPON_IOS_GPON_TRTCM_SCALE, &gponTrtcmConfig)) != 0) {
					break ;
				}
				
				for(i=0 ; i<CONFIG_GPON_MAX_TCONT ; i++) {
					gponTrtcmParams.channel = i ;
					gponTrtcmParams.cirValue = 0 ;
					gponTrtcmParams.cbsUnit = 0 ;
					gponTrtcmParams.pirValue = 0 ;
					gponTrtcmParams.pbsUnit = 0 ;
					XMCS_IOCTL_SDI(GPON_IOS_GPON_TRTCM_PARAMS, &gponTrtcmParams) ;
				}
			} 	
			if(pMsg->Mask & MASK_GponTrtcmParams) {
				if(pObj->GponTrtcm[0].Channel >= CONFIG_GPON_MAX_TCONT) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				if(pObj->GponTrtcm[0].CIRValue > pObj->GponTrtcm[0].PIRValue) {	
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				gponTrtcmParams.channel = pObj->GponTrtcm[0].Channel ;
				gponTrtcmParams.cirValue = pObj->GponTrtcm[0].CIRValue ;
				gponTrtcmParams.cbsUnit = pObj->GponTrtcm[0].CBSUnit ;
				gponTrtcmParams.pirValue = pObj->GponTrtcm[0].PIRValue ;
				gponTrtcmParams.pbsUnit = pObj->GponTrtcm[0].PBSUnit ;
				if((ret = XMCS_IOCTL_SDI(GPON_IOS_GPON_TRTCM_PARAMS, &gponTrtcmParams)) != 0) {
					break ;
				}
			} 
			break ;
		case MACRO_CMDID_GET(GponDbaConfig) :
			ret = XMCS_IOCTL_SDI(GPON_IOG_GPON_TRTCM_CONFIG, &gponTrtcmConfig) ;
			if(ret != 0) {
				break ;
			}
			pObj->GponTrtcmMode = (gponTrtcmConfig.trtcmMode==XPON_ENABLE) ? ENUM_ENABLE : ENUM_DISABLE ;
			pObj->GponTrtcmScale = gponTrtcmConfig.trtcmScale ;
		
			for(i=0 ; i<CONFIG_GPON_MAX_TCONT ; i++) {
				gponTrtcmParams.channel = i ;
				ret = XMCS_IOCTL_SDI(GPON_IOG_GPON_TRTCM_PARAMS, &gponTrtcmParams) ;
				if(ret != 0) {
					break ;
				}
				pObj->GponTrtcm[i].Channel = i ;
				pObj->GponTrtcm[i].CIRValue = gponTrtcmParams.cirValue;
				pObj->GponTrtcm[i].CBSUnit = gponTrtcmParams.cbsUnit ;
				pObj->GponTrtcm[i].PIRValue = gponTrtcmParams.pirValue ;
				pObj->GponTrtcm[i].PBSUnit = gponTrtcmParams.pbsUnit ;
			}
			break ;
    	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(GponDbaConfig, ret) ;
}

/***********************************************************************************
MACRO_CMD_STRUCT(GponTodConfig)
{
	MGR_U32 			SFCounter ;
	MGR_U32 			Second ;
	MGR_U32 			Nanosecond ;
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(GponTodConfig)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(GponTodConfig) *pObj = &pMsg->u.oGponTodConfig ;
	struct XMCS_GponTodCfg_S gponTodCfg ;
	
	switch(pMsg->CmdId) {
		//case MACRO_CMDID_CREATE(GponTodConfig) :		
		//	break ;
		//case MACRO_CMDID_DELETE(GponTodConfig) :		
		//	break ;
		case MACRO_CMDID_SET(GponTodConfig) :
			gponTodCfg.superframe = pObj->SFCounter ;
			gponTodCfg.sec = pObj->Second ;
			gponTodCfg.nanosec = pObj->Nanosecond ;
			
			ret = XMCS_IOCTL_SDI(GPON_IOS_TOD_CFG, &gponTodCfg) ;
			break ;
		case MACRO_CMDID_GET(GponTodConfig) :
			ret = XMCS_IOCTL_SDI(GPON_IOG_CURRENT_TOD, &gponTodCfg) ;
			if(ret == 0) {
				pObj->SFCounter = 0 ;
				pObj->Second = gponTodCfg.sec ;
				pObj->Nanosecond = gponTodCfg.nanosec ;				
			}
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(GponTodConfig, ret) ;
}


/***********************************************************************************
MACRO_CMD_STRUCT(GponGemPortConfig)
{
	struct {
		MGR_GponGemType_t	GemType ;
		MGR_U16				GemPortId ;
		MGR_U16				AllocId ;		//just for create command
		MGR_U8				IfIndex ;		//assign/unassign MAC bridge port
		MGR_Enable_t		Loopback ;		//enable/disable loopback
		MGR_Enable_t		Encryption ;	//just for get command
	} GemPort[CONFIG_GPON_MAX_GEMPORT] ;
	MGR_U16		EntryNum ;
} ;
MACRO_CMD_MASK(GponGemPortConfig)
{
	MASK_AssignMacBridge				= (1<<0) ,
	MASK_UnassignMacBridge				= (1<<1) ,
	MASK_Loopback						= (1<<2)
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(GponGemPortConfig)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(GponGemPortConfig) *pObj = &pMsg->u.oGponGemPortConfig ;
	int i ;
	struct XMCS_GemPortCreate_S gponGemCreate ;
	struct XMCS_GemPortInfo_S gponGemInfo ;
	struct XMCS_GemPortAssign_S gponGemAssign ;
	struct XMCS_GemPortLoopback_S gponGemLoopback ;
	
	if(pObj->EntryNum >= CONFIG_GPON_MAX_GEMPORT) {
		MACRO_PROC_RETURN(GponGemPortConfig, EXEC_ERR_PARAM_INVAL) ;
	}
	
	switch(pMsg->CmdId) {
		case MACRO_CMDID_CREATE(GponGemPortConfig) :	
			for(i=0 ; i<pObj->EntryNum ; i++) {
				if(pObj->GemPort[i].GemPortId > CONST_MAX_GEMPORT_ID) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				
				if(pObj->GemPort[i].GemType == ENUM_GPON_GEM_TYPE_MULTICAST) {
					gponGemCreate.gemType = GPON_MULTICAST_GEM ;
					gponGemCreate.gemPortId = pObj->GemPort[i].GemPortId ;
				} else {
					if(pObj->GemPort[i].AllocId > CONST_MAX_TCONT_ID) {
						ret = EXEC_ERR_PARAM_INVAL ;
						break ;
					}
	
					gponGemCreate.gemType = GPON_UNICAST_GEM ;
					gponGemCreate.gemPortId = pObj->GemPort[i].GemPortId ;
					gponGemCreate.allocId = pObj->GemPort[i].AllocId ;
				}
				ret = XMCS_IOCTL_SDI(IF_IOS_GEMPORT_CREATE, &gponGemCreate) ;
				if(ret != 0) {
					break ;
				}
			}
			break ;
		case MACRO_CMDID_DELETE(GponGemPortConfig) :
			for(i=0 ; i<pObj->EntryNum ; i++) {
				if(pObj->GemPort[i].GemPortId > CONST_MAX_GEMPORT_ID) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
	
				ret = XMCS_IOCTL_SDI(IF_IOS_GEMPORT_REMOVE, pObj->GemPort[i].GemPortId) ;
				if(ret != 0) {
					break ;
				}
			}
			break ;
		case MACRO_CMDID_SET(GponGemPortConfig) :
			if(pMsg->Mask & MASK_AssignMacBridge) {
				gponGemAssign.entryNum = 0 ;
				for(i=0 ; i<pObj->EntryNum ; i++) {
					if(pObj->GemPort[i].GemPortId<=CONST_MAX_GEMPORT_ID && pObj->GemPort[i].IfIndex<CONST_MAX_BRIDGE_PORT) {
						gponGemAssign.gemPort[gponGemAssign.entryNum].id = pObj->GemPort[i].GemPortId ;
						gponGemAssign.gemPort[gponGemAssign.entryNum].ani = pObj->GemPort[i].IfIndex ;
						gponGemAssign.entryNum++ ;
					}
				}
				if((ret = XMCS_IOCTL_SDI(IF_IOS_GEMPORT_ASSIGN, &gponGemAssign)) != 0) {
					break ;
				}
			} 
			if(pMsg->Mask & MASK_UnassignMacBridge) {
				gponGemAssign.entryNum = 0 ;
				for(i=0 ; i<pObj->EntryNum ; i++) {
					if(pObj->GemPort[i].GemPortId <= CONST_MAX_GEMPORT_ID) {
						gponGemAssign.gemPort[gponGemAssign.entryNum].id = pObj->GemPort[i].GemPortId ;
						gponGemAssign.entryNum++ ;
					}
				}
				if((ret = XMCS_IOCTL_SDI(IF_IOS_GEMPORT_UNASSIGN, &gponGemAssign)) != 0) {
					break ;
				}
			}
			if(pMsg->Mask & MASK_Loopback) {
				for(i=0 ; i<pObj->EntryNum ; i++) {
					if(pObj->GemPort[i].GemPortId > CONST_MAX_GEMPORT_ID) {
						ret = EXEC_ERR_PARAM_INVAL ;
						break ;
					}
					
					gponGemLoopback.gemPortId = pObj->GemPort[i].GemPortId ;
					gponGemLoopback.loopback = (pObj->GemPort[i].Loopback==ENUM_ENABLE) ? XPON_ENABLE : XPON_DISABLE ;
					if((ret = XMCS_IOCTL_SDI(IF_IOS_GEMPORT_LOOPBACK, &gponGemLoopback)) != 0) {
						break ;
					}
				}
			}
			break ;
		case MACRO_CMDID_GET(GponGemPortConfig) :
			ret = XMCS_IOCTL_SDI(IF_IOG_GEMPORT_INFO, &gponGemInfo) ;
			if(ret == 0) {
				if(gponGemInfo.entryNum > CONFIG_GPON_MAX_GEMPORT) {
					ret = EXEC_ERR ;
				} else {
					for(i=0 ; i<gponGemInfo.entryNum ; i++) {
						pObj->GemPort[i].GemType = (gponGemInfo.info[i].gemType==GPON_MULTICAST_GEM)?ENUM_GPON_GEM_TYPE_MULTICAST:ENUM_GPON_GEM_TYPE_UNICAST ;	
						pObj->GemPort[i].GemPortId = gponGemInfo.info[i].gemPortId ;	
						pObj->GemPort[i].AllocId = gponGemInfo.info[i].allocId ;	
						pObj->GemPort[i].IfIndex = gponGemInfo.info[i].aniIdx ;	
						pObj->GemPort[i].Loopback = (gponGemInfo.info[i].lbMode==XPON_ENABLE) ? ENUM_ENABLE : ENUM_DISABLE ;	
						pObj->GemPort[i].Encryption = (gponGemInfo.info[i].enMode==XPON_ENABLE) ? ENUM_ENABLE : ENUM_DISABLE ;
					}
					pObj->EntryNum = gponGemInfo.entryNum ;
				}
			}
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(GponGemPortConfig, ret) ;
}

/***********************************************************************************
MACRO_CMD_STRUCT(GponTcontStatus)
{
	struct {
		MGR_U16			AllocId ;
		MGR_U8			Channel ;
	} Tcont[CONFIG_GPON_MAX_TCONT] ;
	MGR_U16		EntryNum ;
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(GponTcontStatus)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(GponTcontStatus) *pObj = &pMsg->u.oGponTcontStatus ;
	struct XMCS_TcontInfo_S gponTcontInfo ;
	int i ;
	
	switch(pMsg->CmdId) {
		//case MACRO_CMDID_CREATE(GponTcontStatus) :		
		//	break ;
		//case MACRO_CMDID_DELETE(GponTcontStatus) :
		//	break ;
		//case MACRO_CMDID_SET(GponTcontStatus) :
		//	break ;
		case MACRO_CMDID_GET(GponTcontStatus) :
			ret = XMCS_IOCTL_SDI(IF_IOG_TCONT_INFO, &gponTcontInfo) ;
			if(ret == 0) {
				if(gponTcontInfo.entryNum > CONFIG_GPON_MAX_TCONT) {
					ret = EXEC_ERR ;
				} else {
					for(i=0 ; i<gponTcontInfo.entryNum ; i++) {
						pObj->Tcont[i].AllocId = gponTcontInfo.info[i].allocId ;
						pObj->Tcont[i].Channel = gponTcontInfo.info[i].channel ;
					}
					pObj->EntryNum = gponTcontInfo.entryNum ;
				}
			}
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(GponTcontStatus, ret) ;
}

/***********************************************************************************
MACRO_CMD_STRUCT(GponGemCounter)
{
	MGR_U16 			GemPortId ;
	MGR_U32				RxGemFrameH ;
	MGR_U32				RxGemFrameL ;
	MGR_U32				RxGemPayloadH ;
	MGR_U32				RxGemPayloadL ;
	MGR_U32				TxGemFrameH ;
	MGR_U32				TxGemFrameL ;	
	MGR_U32				TxGemPayloadH ;	
	MGR_U32				TxGemPayloadL ;		
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(GponGemCounter)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(GponGemCounter) *pObj = &pMsg->u.oGponGemCounter ;
	struct XMCS_GponGemCounter_S gponGemCounter ;
	
	switch(pMsg->CmdId) {
		//case MACRO_CMDID_CREATE(GponGemCounter) :		
		//	break ;
		//case MACRO_CMDID_DELETE(GponGemCounter) :
		//	break ;
		//case MACRO_CMDID_SET(GponGemCounter) :
		//	break ;
		case MACRO_CMDID_GET(GponGemCounter) :
			memset(&gponGemCounter, 0, sizeof(struct XMCS_GponGemCounter_S)) ;
			gponGemCounter.gemPortId = pObj->GemPortId ;
			ret = XMCS_IOCTL_SDI(GPON_IOG_GEM_COUNTER, &gponGemCounter) ;
			pObj->RxGemFrameH = gponGemCounter.rxGemFrameH;
			pObj->RxGemFrameL = gponGemCounter.rxGemFrameL;
			pObj->RxGemPayloadH = gponGemCounter.rxGemPayloadH;
			pObj->RxGemPayloadL = gponGemCounter.rxGemPayloadL;
			pObj->TxGemFrameH = gponGemCounter.txGemFrameH;
			pObj->TxGemFrameL = gponGemCounter.txGemFrameL;
			pObj->TxGemPayloadH = gponGemCounter.txGemPayloadH;
			pObj->TxGemPayloadL = gponGemCounter.txGemPayloadL;			
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(GponGemCounter, ret) ;
}

