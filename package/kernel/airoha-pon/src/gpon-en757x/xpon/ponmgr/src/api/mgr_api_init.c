#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "api/mgr_api.h"
#include "common/mgr_pthread.h"

extern MGR_Ret imgr_start(void) ;
struct PONAPI_AppInfo_S	appInfo = {0, 0, 0}  ;
int gXponMode = 0; /* 0, auto, 1, gpon, 2, epon */

/**************************************************************************************************
**************************************************************************************************/
MGR_Ret ponapi_init(int xponmode) 
{
	MGR_Ret ret ;

	/* set xpon mode */
	if (xponmode != -1)
		gXponMode = xponmode;

	appInfo.pid = getpid() ;
	if(!appInfo.trapFlag) {
		ret = imgr_start() ;
		if(ret != EXEC_OK) {
			DEBUG_PERROR("Imgr initialization failed, errno: %d\n", ret) ;
			return ret ;
		}
		
		appInfo.trapFlag = 1 ;
	}

	return EXEC_OK ;
}

/**************************************************************************************************
**************************************************************************************************/
MGR_Ret ponapi_destory(void)
{
	return EXEC_OK ;
}

