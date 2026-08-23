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

/************************************************************************
***********************************************************************/
MGR_Ret sample_for_omci_trap_handler(MACRO_CMD_STRUCT(SystemTrapInfo) *pTrapInfo)
{
	MGR_Ret ret = EXEC_OK ;
	
	switch(pTrapInfo->trapIndex) {
		case GPON_TRAP_DETECT_LOS:
			printf("receive GPON LOS trap message\n") ;
			break ;
		case GPON_TRAP_PHY_READY:
			printf("receive GPON PHY_READY trap message\n") ;
			break ;
		case GPON_TRAP_DEACTIVATE:
			printf("receive GPON DEACTIVATE trap message\n") ;
			break ;
		case GPON_TRAP_ACTIVATE:
			printf("receive GPON ACTIVATE trap message\n") ;
			break ;
		default:
			printf("receive a unknown trap message, %x\n", pTrapInfo->trapIndex) ;
			ret = EXEC_ERR_UNKNOWN_TRAP_GROUP ;
			break ;
	}
	
	return ret ;
}	

/********************************************************************************************
********************************************************************************************/
void sample_for_omci_destory(void) 
{
	ponapi_trap_destory() ;
		
	exit(0) ;
}


/********************************************************************************************
********************************************************************************************/
int main(int argc, char **const argv)
{
	if(ponapi_trap_init(TASK_TYPE_OMCI, &sample_for_omci_trap_handler) != EXEC_OK) {
		printf("Trap handler initial error\n") ;
		exit(0) ;
	}

	while(1) {
		sleep(1) ;
	}
	
	sample_for_omci_destory() ;
	
	return 0 ;
}

