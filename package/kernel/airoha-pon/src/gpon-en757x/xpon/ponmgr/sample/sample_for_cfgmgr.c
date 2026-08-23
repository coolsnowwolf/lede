#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <pwd.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "api/mgr_api.h"

/********************************************************************************************
MACRO_CMD_MASK(PonTestCmd)
{
	MASK_SetValue1						= (1<<0) ,
	MASK_SetValue2	 					= (1<<1) ,
	MASK_SetValue3		 				= (1<<2) ,
	MASK_GenEvent						= (1<<3) 
} ;
MACRO_CMD_STRUCT(PonTestCmd)
{
	MGR_U32 value1 ;
	MGR_U32 value2 ;
	MGR_U32	value3 ;
} ;
********************************************************************************************/
int sample_for_cfgmgr_set_command(void) 
{
#if 0
	MACRO_CMD_STRUCT(PonTestCmd) setEntry, getEntry ;
	MGR_Mask mask = 0 ;

	/*********************************
	 example for set command
	*********************************/	
	setEntry.value1 = 10 ;
	setEntry.value2 = 20 ;
	setEntry.value3 = 30 ;
	mask = (MASK_SetValue1|MASK_SetValue2|MASK_SetValue3) ;
	if(PonApi_PonTestCmd_Set(mask, &setEntry) != EXEC_OK) {
		printf("Set the test command failed\n\n") ;
		return -1 ;
	}


	/*********************************
	 example for get command
	*********************************/	
	if(PonApi_PonTestCmd_Get(&getEntry) != EXEC_OK) {
		printf("Get the test command failed\n\n") ;
		return -1 ;
	}
	
	printf("===>Get Value 1: %d\n", getEntry.value1) ;
	printf("===>Get Value 2: %d\n", getEntry.value2) ;
	printf("===>Get Value 3: %d\n", getEntry.value3) ;

	/****************************************
	 example for get value from database
	****************************************/
	printf("===>Get Value 1: %d\n", gpSysDbAddr->pon.ponTestCmd.value1) ;
	printf("===>Get Value 2: %d\n", gpSysDbAddr->pon.ponTestCmd.value2) ;
	printf("===>Get Value 3: %d\n", gpSysDbAddr->pon.ponTestCmd.value3) ;
#endif /* 0 */
	return 0 ;
}

/********************************************************************************************
********************************************************************************************/
int main(int argc, char **const argv)
{
	if(ponapi_init() != EXEC_OK) {
		printf("CLI initial error\n") ;
		exit(0) ;
	}
	
	while(1) {	
		sample_for_cfgmgr_set_command() ;
		sleep(1) ;
	}
	
	ponapi_destory() ;
	
	return 0 ;
}

