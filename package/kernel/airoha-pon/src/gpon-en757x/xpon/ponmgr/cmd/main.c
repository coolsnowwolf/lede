#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "cmd_errno.h"
#include "core/cmgr/cmgr_init.h"
#include "api/mgr_api.h"

extern void ponmgrProcessSystemCommand(int argc, char** argv) ;
extern void ponmgrProcessPWanCommand(int argc, char** argv) ;
#ifdef TCSUPPORT_WAN_GPON
	extern void ponmgrProcessGponCommand(int argc, char** argv) ;
#endif /* TCSUPPORT_WAN_GPON */


/********************************************************************************************
********************************************************************************************/
int main(int argc, char **const argv)
{
	init_compileoption_val();
	if(cmgr_init() != EXEC_OK) {
		printf("Config manager initial failed.\n") ;
		exit(0) ;
	}
	
	if(argc>=4 && !strcmp(argv[1], "system")) {
		ponmgrProcessSystemCommand(argc, argv) ;
	} else if(argc>=4 && !strcmp(argv[1], "pwan")) {
		ponmgrProcessPWanCommand(argc, argv) ;
	} else 		
#ifdef TCSUPPORT_WAN_GPON
	if(argc>=4 && !strcmp(argv[1], "gpon")) {
		ponmgrProcessGponCommand(argc, argv) ;
	} else 
#endif /* TCSUPPORT_WAN_GPON */
	{
		printf("CMD Failed: input command '%s' failed.\n", argv[1]) ;	
	}

	cmgr_destroy() ;
	
	return 0 ;
}

