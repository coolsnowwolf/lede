#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "api/mgr_api.h"
#include "cmd_cmds.h"
#include "cmd_enum.h"
#include "cmd_const.h"


/*********************************************************************************************
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
*********************************************************************************************/
void cmdSystemGetLinkMode(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(SysLinkConfig) sysLinkCfg ;
	
	if(PonApi_SysLinkConfig_Get(&sysLinkCfg) != EXEC_OK) {
		printf("Exec. Failed: Get the 'SysLinkConfig' command failed\n") ;
		return ;
	}
	
	printf("Wan Link Start: %s\n", (sysLinkCfg.LinkStart) ? "Start" : "Stop") ;
	printf("Wan Detection Mode: ") ;
	if(sysLinkCfg.WanDetectMode == ENUM_SYSTEM_WAN_DETECTION_AUTO) {
		printf(" Auto Detection\n") ;
	} else if(sysLinkCfg.WanDetectMode == ENUM_SYSTEM_WAN_DETECTION_GPON) {
		printf(" GPON Mode\n") ;
	} else {
		printf(" EPON Mode\n") ;
	}

	printf("WAN Link Status: ") ;
	if(sysLinkCfg.LinkStatus == ENUM_SYSTEM_LINK_GPON) {
		printf(" GPON Link\n") ;
	} else if(sysLinkCfg.LinkStatus == ENUM_SYSTEM_LINK_EPON) {
		printf(" EPON Link\n") ;
	} else {
		printf(" Link Off\n") ;
	}
}


/*******************************************************************************************
*******************************************************************************************/
/*****************************************************************************
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
*****************************************************************************/
void cmdSystemSetStartMode(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(SysLinkConfig) sysLinkCfg ;
	
	if(!strcmp(argv[0], "start")) {
		sysLinkCfg.LinkStart = ENUM_ENABLE ;
	} else if(!strcmp(argv[0], "stop")) {
		sysLinkCfg.LinkStart = ENUM_DISABLE ;
	} else {
		printf("Input Error: Connection start mode error.\n") ;
		return ;
	}
	
	if(PonApi_SysLinkConfig_Set((MASK_SysLinkStart), &sysLinkCfg) != EXEC_OK) {
		printf("Exec. Failed: Set the 'SysLinkConfig' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;	
}

/*****************************************************************************
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
*****************************************************************************/
void cmdSystemSetWanDetection(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(SysLinkConfig) sysLinkCfg ;
	
	if(!strcmp(argv[0], "auto")) {
		sysLinkCfg.WanDetectMode = ENUM_SYSTEM_WAN_DETECTION_AUTO ;
	} else if(!strcmp(argv[0], "gpon")) {
		sysLinkCfg.WanDetectMode = ENUM_SYSTEM_WAN_DETECTION_GPON ;
	} else if(!strcmp(argv[0], "epon")) {
		sysLinkCfg.WanDetectMode = ENUM_SYSTEM_WAN_DETECTION_EPON ;
	} else {
		printf("Input Error: WAN detection mode error.\n") ;
		return ;
	}
	
	if(PonApi_SysLinkConfig_Set((MASK_SysWanDetectMode), &sysLinkCfg) != EXEC_OK) {
		printf("Exec. Failed: Set the 'SysLinkConfig' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;	
}


void cmdSystemClearConfig(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(SysClearConfig) sysClearCfg ;
	if(!strcmp(argv[0], "xpon")) {
		sysClearCfg.ClearType = ENUM_SYSTEM_CLEAR_XPON ;
	} else {
		printf("Input Error: Clear mode error.\n") ;
		return ;
	}	
	if(PonApi_SysClearConfig_Set((MASK_SysClearXpon), &sysClearCfg) != EXEC_OK) {
		printf("Exec. Failed: Set the 'SysClearConfig' command failed\n\n") ;
		return ;
	}	
	printf("Exec. Successful\n") ;	
}


void cmdSystemGetOnuType(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(SysOnuType) sysOnuType ;
	
	if(PonApi_SysOnuType_Get(&sysOnuType) != EXEC_OK) {
		printf("Exec. Failed: Get the 'SysOnuType' command failed\n") ;
		return ;
	}

	printf("ONU type: ") ;

	if(sysOnuType.OnuType == ENUM_SYSTEM_ONU_TYPE_UNKNOWN){
		printf(" unknown\n");
	}else if(sysOnuType.OnuType == ENUM_SYSTEM_ONU_TYPE_SFU){
		printf(" sfu\n");
	}else if(sysOnuType.OnuType == ENUM_SYSTEM_ONU_TYPE_HGU){
		printf(" hgu\n");
	} else {
		printf(" error,ret=%d \n",sysOnuType.OnuType);
	}
}



/********************************************************************************************
********************************************************************************************/
void ponmgrProcessSystemCommand(int argc, char** argv) 
{
	int i, flag ;
	
	if(!strcmp(argv[2], "set")) {
		flag = kSysCmdSet ;
	} else if(!strcmp(argv[2], "get")) {
		flag = kSysCmdGet ;
	} else if(!strcmp(argv[2], "create")) {
		flag = kSysCmdCreate ;
	} else if(!strcmp(argv[2], "delete")) {
		flag = kSysCmdDelete ;
	} else {
		printf("CMD Failed: input command '%s' failed.\n", argv[2]) ;	
		return ;
	}

	for(i=0 ; i<gSystemCmdNum ; i++) {
		if((gSystemCmd[i].flags&flag) && !strcmp(argv[3], gSystemCmd[i].name)) {
			if(argc>=(gSystemCmd[i].minArgs+4) && argc<=(gSystemCmd[i].maxArgs+4)) {
				gSystemCmd[i].proc(argc-4, &argv[4]) ;
				return ;
			} else {
				printf("CMD Failed: input arguments failed.\n") ;	
				return ;
			}
		}
	}

	printf("CMD Failed: no such command, '%s'.\n", argv[3]) ;	
	return ;
}

