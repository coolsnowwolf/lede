#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "common/mgr_ipc.h"
#include "common/mgr_cmd.h"

#include "core/pmgr/pmgr.h"


/*****************************************************************************************
*****************************************************************************************/
MGR_Ret pmgr_cmd_shunter(MGR_CmdMsg_T *pMsg)
{
	MGR_Ret ret ;
	MGR_U16 cmdId = pMsg->CmdId/CONST_GROUP_CMD_NUM ;
	
	switch(cmdId) {
		MACRO_CMD_PROC_SWITCH(PhyFecCounter) ;
		MACRO_CMD_PROC_SWITCH(PhyFrameCounter) ;
		
		default:
			ret = pMsg->Result = EXEC_ERR_NO_SUCH_CMD ; 
			DEBUG_PERROR("Paraer the pmgr command %d error, errno:%d.\n", cmdId, ret) ;
			break ;
	}

	return ret ;	
}

