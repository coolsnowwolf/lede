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
typedef enum {
	ENUM_SYSTEM_WAN_DETECTION_AUTO	 		= XMCS_IF_WAN_DETECT_MODE_AUTO,
	ENUM_SYSTEM_WAN_DETECTION_GPON			= XMCS_IF_WAN_DETECT_MODE_GPON,
	ENUM_SYSTEM_WAN_DETECTION_EPON			= XMCS_IF_WAN_DETECT_MODE_EPON
} MGR_SysWanDetectMode_t ;

typedef enum {
	ENUM_SYSTEM_LINK_OFF 					= XMCS_IF_WAN_LINK_OFF,
	ENUM_SYSTEM_LINK_GPON					= XMCS_IF_WAN_LINK_GPON,
	ENUM_SYSTEM_LINK_EPON					= XMCS_IF_WAN_LINK_EPON
} MGR_SysLinkStatus_t ;

MACRO_CMD_MASK(SysLinkConfig)
{	
	MASK_SysLinkStart					= (1<<0) ,
	MASK_SysWanDetectMode				= (1<<1)
} ;
MACRO_CMD_STRUCT(SysLinkConfig)
{
	MGR_Enable_t				LinkStart ;
	MGR_SysWanDetectMode_t		WanDetectMode ;
	MGR_SysLinkStatus_t			LinkStatus ;
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(SysLinkConfig)
{
	MGR_Ret ret = EXEC_ERR_CMD_NOT_SUPPORT ;
	MACRO_CMD_STRUCT(SysLinkConfig) *pObj = &pMsg->u.oSysLinkConfig ;
	struct XMCS_WanLinkConfig_S sysLinkCfg ;
	
	switch(pMsg->CmdId)
	{
		case MACRO_CMDID_SET(SysLinkConfig) :
			if(pMsg->Mask & MASK_SysWanDetectMode) {
				if(pObj->WanDetectMode!=XMCS_IF_WAN_DETECT_MODE_AUTO && pObj->WanDetectMode!=XMCS_IF_WAN_DETECT_MODE_GPON && pObj->WanDetectMode!=XMCS_IF_WAN_DETECT_MODE_EPON) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				} 
				if((ret = XMCS_IOCTL_SDI(IO_IOS_WAN_DETECTION_MODE, pObj->WanDetectMode)) != 0) {
					break ;
				}
			}
			if(pMsg->Mask & MASK_SysLinkStart) {
				sysLinkCfg.linkStart = (pObj->LinkStart==ENUM_DISABLE) ? XPON_DISABLE : XPON_ENABLE ;
				if((ret = XMCS_IOCTL_SDI(IO_IOS_WAN_LINK_START, sysLinkCfg.linkStart)) != 0) {
					break ;
				}
			}
			break ;
			
		case MACRO_CMDID_GET(SysLinkConfig) :
			if((ret = XMCS_IOCTL_SDI(IO_IOG_WAN_LINK_CONFIG, &sysLinkCfg)) != 0) {
				break ;
			}
			pObj->LinkStart = (sysLinkCfg.linkStart==XPON_DISABLE) ? ENUM_DISABLE : ENUM_ENABLE ;
			pObj->WanDetectMode = sysLinkCfg.detectMode ;
			pObj->LinkStatus = sysLinkCfg.linkStatus ;
			break ;
			
 		case MACRO_CMDID_CREATE(SysLinkConfig) :	
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
		case MACRO_CMDID_DELETE(SysLinkConfig) :
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(SysLinkConfig, ret) ;
}

/***********************************************************************************
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(SysOnuType)
{
	MGR_Ret ret = EXEC_ERR_CMD_NOT_SUPPORT ;
	MACRO_CMD_STRUCT(SysOnuType) *pObj = &pMsg->u.oSysOnuType ;
	switch(pMsg->CmdId)
	{
		case MACRO_CMDID_GET(SysOnuType) :
			if((ret = XMCS_IOCTL_SDI(IO_IOG_ONU_TYPE, &pObj->OnuType)) != 0) {
				break ;
			}
			break;
		default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}	  
			
	MACRO_PROC_RETURN(SysOnuType, ret) ;
}
/***********************************************************************************
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(SysClearConfig)
{
	MGR_Ret ret = EXEC_ERR_CMD_NOT_SUPPORT ;
	MACRO_CMD_STRUCT(SysClearConfig) *pObj = &pMsg->u.oSysClearConfig ;
	struct XMCS_ClearConfig_S sysClearCfg ;

	switch(pMsg->CmdId)
	{
		case MACRO_CMDID_SET(SysClearConfig) :
			if(pMsg->Mask & MASK_SysClearXpon) {
				if(pObj->ClearType!=XMCS_IF_CLEAR_TYPE_XPON) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				} 
				if((ret = XMCS_IOCTL_SDI(FDET_IOG_RMMOD_EVENT, pObj->ClearType)) != 0) {
					break ;
				}
			}

			break ;
		default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}	  
	
}



